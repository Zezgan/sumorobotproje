from flask import Flask, render_template, request, jsonify
import serial.tools.list_ports
import serial
import json
import os

app = Flask(__name__)
file_path = "arduino_config.json"
pin_adres_map = {
    "SOL_ON_PIN1_ADRES":1,
    "SOL_ON_PIN2_ADRES":2,
    "SAG_ARKA_PIN1_ADRES":3,
    "SAG_ARKA_PIN2_ADRES":4,
    "SOL_ENABLE_PIN_ADRES":5,
    "SAG_ENABLE_PIN_ADRES":6,
    "ON_LED_PIN_ADRES":7,
    "ARKA_LED_PIN_ADRES":8,
    }

@app.route("/")
def anasayfa():
    return render_template("anasayfa.html")


@app.route("/konfigrasyon")
def konfigrasyon():
    serial_ports_list = [port.device for port in serial.tools.list_ports.comports()]
    serial_ports_dict = {port: port for port in serial_ports_list}
    
    arduino_ports = {
        "D0":0, "D1":1, "D2":2, "D3":3, "D4":4,
        "D5":5, "D6":6, "D7":7, "D8":8, "D9":9,
        "D10":10, "D11":11, "D12":12, "D13":13,
        "A0":"A0", "A1":"A1", "A2":"A2", "A3":"A3", "A4":"A4", "A5":"A5"
    }

    fields = [
        {"label":"EKİP ADI", "type":"text", "name":"ekip_id"},
        {"label":"PORT", "type":"select", "name":"port", "options":serial_ports_dict},
        {"label":"HIZ_ADRES", "type":"select", "name":"hiz_adress", "options":arduino_ports},
        {"label":"MOD_ADRES", "type":"select", "name":"mod_adres", "options":arduino_ports},
        {"label":"SOL_ON_PIN1_ADRES", "type":"select", "name":"sol_on1", "options":arduino_ports},
        {"label":"SOL_ON_PIN2_ADRES", "type":"select", "name":"sol_on2", "options":arduino_ports},
        {"label":"SAG_ARKA_PIN1_ADRES", "type":"select", "name":"sag_arka1", "options":arduino_ports},
        {"label":"SAG_ARKA_PIN2_ADRES", "type":"select", "name":"sag_arka2", "options":arduino_ports},
        {"label":"SOL_ENABLE_PIN_ADRES", "type":"select", "name":"sol_enable", "options":arduino_ports},
        {"label":"SAG_ENABLE_PIN_ADRES", "type":"select", "name":"sag_enable", "options":arduino_ports},
        {"label":"ON_LED_PIN_ADRES", "type":"select", "name":"on_led", "options":arduino_ports},
        {"label":"ARKA_LED_PIN_ADRES", "type":"select", "name":"arka_led", "options":arduino_ports},
        {"label":"MOD SEÇİMİ", "type":"select", "name":"mod_secimi", "options":{"Otomatik Mod":1,"Klavye Mod":2,"Bluetuh Mod":3}},
    ]

    return render_template("konfigrasyon.html", fields=fields)

@app.route("/save_config", methods=["POST"])
def save_config():
    data = request.get_json()
    port = data.get("port")
    baudrate = int(data.get("baudrate", 9600))

    #success, result = arduino_yaz(port, baudrate, data)  # dict string olarak yollanıyor
    success = True
    if success:
        # JSON dosyasına veri kaydetme
        if os.path.exists(file_path):
            with open(file_path, 'r') as f:
                try:
                    existing_data = json.load(f)
                except json.JSONDecodeError:
                    existing_data = []
        else:
            existing_data = []

        existing_data.append(data)

        with open(file_path, 'w') as f:
            json.dump(existing_data, f, indent=4)
        return jsonify({"status": "ok", "message": "Veri kaydedildi"})
    else:
        return jsonify({"status": "error", "message": result}), 500

@app.route("/kayitli_veriler")
def kayitli_veriler():
    try:
        with open(file_path) as f:
            veri = json.load(f)
        return render_template("kayitli-veriler.html", veriler=veri)
    except FileNotFoundError:
        return render_template("kayitli-veriler.html", veriler=[])

@app.route("/update_config", methods=["POST"])
def update_config():
    try:
        content = request.get_json()
        index = int(content.get("index"))
        new_data = content.get("data")

        with open(file_path, 'r') as f:
            data = json.load(f)

        if index < 0 or index >= len(data):
            return jsonify({"status": "error", "message": "Geçersiz index"}), 400

        data[index] = new_data

        with open(file_path, 'w') as f:
            json.dump(data, f, indent=4)

        return jsonify({"status": "ok", "message": "Güncelleme başarılı"})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route("/delete_config", methods=["POST"])
def delete_config():
    try:
        index = int(request.get_json().get("index"))

        with open(file_path, 'r') as f:
            data = json.load(f)

        if index < 0 or index >= len(data):
            return jsonify({"status": "error", "message": "Geçersiz index"}), 400

        data.pop(index)

        with open(file_path, 'w') as f:
            json.dump(data, f, indent=4)

        return jsonify({"status": "ok", "message": "Silme başarılı"})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route("/upload_config", methods=["POST"])
def upload_config():
    try:
        content = request.get_json()
        port = content.get("port")
        baudrate = int(content.get("baudrate", 9600))
        
        status,error = konfigrasyon_yaz(port,baudrate,content)

        return jsonify({"status": "ok", "message": "Yükleme tamamlandı."})

    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

def arduino_yaz(port, baudrate, data):
    import serial
    try:
        with serial.Serial(port, baudrate, timeout=2) as ser:
            ser.write(data.encode('utf-8'))
    except Exception as e:
        print(f"[Arduino Yaz Hatası] {e}")

@app.route("/kontrol")
def kontrol():
    return render_template("kontrol.html")

@app.route("/kontrol_komut", methods=["POST"])
def kontrol_komut():
    try:
        content = request.get_json()
        komut = content.get("komut")
        port = content.get("port")  # portu kullanıcı seçecek
        baudrate = int(content.get("baudrate", 9600))

        if not komut:
            return jsonify({"status": "error", "message": "Komut eksik"})

        data_str = f"{komut}"
        arduino_yaz(port, baudrate, data_str)

        return jsonify({"status": "ok", "message": f"{komut} komutu gönderildi"})
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route("/get_ports")
def get_ports():
    try:
        ports = serial.tools.list_ports.comports()
        port_list = [
            {
                "device": port.device,
                "description": port.description
            }
            for port in ports
        ]
        return jsonify(port_list)
    except Exception as e:
        return jsonify([])  # boş liste döndür

def konfigrasyon_yaz(port, baudrate, data):
    try:
        for key, value in data.items():
            if key in pin_adres_map:
                pin_tipi = pin_adres_map[key]
                pin_degeri = value
                # Format: P,8,13 gibi örnek
                data_str = f"P,{pin_tipi},{pin_degeri}"
                arduino_yaz(port, baudrate, data_str)
        return True, None
    except Exception as e:
        return False, str(e)

def arduino_yaz(port, baudrate, data):
    try:
        with serial.Serial(port, baudrate, timeout=2) as ser:
            ser.write(data.encode('utf-8'))
    except Exception as e:
        print(f"Yazma Hatası: {e}")

def arduino_oku(port, baudrate):
    try:
        with serial.Serial(port, baudrate, timeout=2) as ser:
            if ser.in_waiting > 0:
                data = ser.readline().decode('utf-8').strip()
                return {"status": "ok", "message": data}
            else:
                return {"status": "ok", "message": "Veri bekleniyor..."}
    except Exception as e:
        return {"status": "error", "message": str(e)}

if __name__ == '__main__':
    app.run(debug=True)