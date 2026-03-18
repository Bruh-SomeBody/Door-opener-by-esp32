#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h> // Library for saving data to ESP32 flash memory

// --- WI-FI SETTINGS ---
const char* ssid = "ssid";
const char* password = "password";

// --- ADMINISTRATOR SETTINGS (for control panel access only) ---
const char* adminUser = "adminlogin";
const char* adminPass = "adminpass";

// Variables for storing regular user credentials (loaded from memory)
String currentLogin;
String currentPass;

// Pins and objects
const int RELAY_PIN = 13;
WebServer server(80); 
Preferences preferences; // Object for working with flash memory

// Variables for timer and Wi-Fi
bool doorIsOpening = false;
unsigned long doorTriggerTime = 0;
const unsigned long OPEN_DURATION = 1000;
unsigned long previousWifiMillis = 0;
const unsigned long WIFI_CHECK_INTERVAL = 10000;

// --- HTML: MAIN PAGE (For teachers) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="uk">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Відчинятор 4.0</title>
  <style>
    body { background-color: #f4f4f9; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; height: 100vh; display: flex; flex-direction: column; align-items: center; justify-content: center; }
    h1 { font-size: 2.2rem; color: #333; text-align: center; margin-bottom: 30px; }
    button { width: 85%; max-width: 400px; padding: 30px 0; font-size: 2rem; font-weight: bold; background-color: #4CAF50; color: white; border: none; border-radius: 25px; cursor: pointer; box-shadow: 0 8px 15px rgba(0,0,0,0.1); transition: all 0.3s ease 0s; outline: none; }
    button:active { background-color: #45a049; transform: translateY(4px); box-shadow: 0 4px 7px rgba(0,0,0,0.1); }
    button:disabled { background-color: #9e9e9e; cursor: not-allowed; transform: none; box-shadow: none; }
    #status { margin-top: 25px; font-size: 1.5rem; font-weight: bold; color: #4CAF50; height: 30px; }
    .lock-icon { font-size: 3rem; margin-bottom: 10px; }
  </style>
</head>
<body>
  <div class="lock-icon">🔒</div>
  <h1>Відчинятор дверей</h1>
  <button id="openBtn" onclick="openDoor()">Відчинити двері</button>
  <div id="status"></div>

  <script>
    function openDoor() {
      const btn = document.getElementById('openBtn');
      const status = document.getElementById('status');
      
      btn.disabled = true;
      status.style.color = '#333';
      status.innerText = 'Надсилаємо сигнал...';
      
      fetch('/open')
        .then(response => {
          if(response.ok) {
            status.style.color = '#4CAF50';
            status.innerText = 'Двері відчинено!';
            setTimeout(() => { status.innerText = ''; btn.disabled = false; }, 2000);
          } else if (response.status === 401) {
             window.location.reload();
          } else { throw new Error('Server error'); }
        })
        .catch(err => {
          status.style.color = 'red';
          status.innerText = 'Помилка мережі!';
          btn.disabled = false;
        });
    }
  </script>
</body>
</html>
)rawliteral";

// --- HTML: ADMIN PANEL (Only for you) ---
const char admin_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="uk">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Адмін-панель</title>
  <style>
    body { background-color: #2c3e50; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; height: 100vh; display: flex; flex-direction: column; align-items: center; justify-content: center; color: white; }
    .admin-box { background-color: #34495e; padding: 40px; border-radius: 15px; box-shadow: 0 10px 20px rgba(0,0,0,0.3); width: 85%; max-width: 350px; }
    h2 { text-align: center; margin-top: 0; }
    label { font-size: 1.1rem; display: block; margin-bottom: 8px; }
    input { width: 100%; padding: 12px; margin-bottom: 20px; border: none; border-radius: 8px; box-sizing: border-box; font-size: 1rem; }
    button { width: 100%; padding: 15px; background-color: #e74c3c; color: white; border: none; border-radius: 8px; font-size: 1.2rem; font-weight: bold; cursor: pointer; transition: 0.2s; }
    button:active { background-color: #c0392b; }
    .success { display: none; background: #2ecc71; padding: 10px; border-radius: 8px; text-align: center; margin-bottom: 20px; font-weight: bold; }
  </style>
</head>
<body>
  <div class="admin-box">
    <h2>⚙️ Налаштування доступу</h2>
    <form action="/update_creds" method="POST">
      <label>Новий логін для вчителів:</label>
      <input type="text" name="new_login" required placeholder="Введіть логін">
      
      <label>Новий пароль для вчителів:</label>
      <input type="text" name="new_pass" required placeholder="Введіть пароль">
      
      <button type="submit">Зберегти зміни</button>
    </form>
  </div>
</body>
</html>
)rawliteral";


void setup() {
  Serial.begin(115200);
  
  digitalWrite(RELAY_PIN, LOW);
  pinMode(RELAY_PIN, OUTPUT);

  // Initialize memory (create "auth" namespace)
  preferences.begin("auth", false);
  // Load saved credentials. If none exist (first run), default to "admin" and "12345678"
  currentLogin = preferences.getString("login", "admin");
  currentPass = preferences.getString("pass", "12345678");
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 

  // --- 1. MAIN PAGE (Access via dynamic user password) ---
  server.on("/", HTTP_GET, []() {
    if (!server.authenticate(currentLogin.c_str(), currentPass.c_str())) {
      return server.requestAuthentication();
    }
    server.send_P(200, "text/html", index_html);
  });

  // --- 2. RELAY ENDPOINT (Access via dynamic user password) ---
  server.on("/open", HTTP_GET, []() {
    if (!server.authenticate(currentLogin.c_str(), currentPass.c_str())) {
      return server.requestAuthentication();
    }
    digitalWrite(RELAY_PIN, HIGH);
    doorIsOpening = true;
    doorTriggerTime = millis();
    server.send(200, "text/plain", "OK");
  });

  // --- 3. ADMIN PANEL (Access via secret admin password) ---
  server.on("/admin", HTTP_GET, []() {
    if (!server.authenticate(adminUser, adminPass)) {
      return server.requestAuthentication();
    }
    server.send_P(200, "text/html", admin_html);
  });

  // --- 4. HANDLE PASSWORD CHANGE (Access via secret admin password) ---
  server.on("/update_creds", HTTP_POST, []() {
    if (!server.authenticate(adminUser, adminPass)) {
      return server.requestAuthentication();
    }
    
    // Check if form data was received
    if (server.hasArg("new_login") && server.hasArg("new_pass")) {
      currentLogin = server.arg("new_login");
      currentPass = server.arg("new_pass");
      
      // Save new credentials to flash memory
      preferences.putString("login", currentLogin);
      preferences.putString("pass", currentPass);
      
      // Send response and back button
      String response = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta charset='UTF-8'></head>";
      response += "<body style='background:#2c3e50; color:white; font-family:sans-serif; text-align:center; padding-top:20%;'>";
      response += "<h2>✅ Дані успішно оновлено!</h2>";
      response += "<p>Новий логін: <b>" + currentLogin + "</b></p>";
      response += "<p>Новий пароль: <b>" + currentPass + "</b></p>";
      response += "<br><a href='/' style='padding:15px 30px; background:#2ecc71; color:white; text-decoration:none; border-radius:8px; font-weight:bold;'>На головну</a>";
      response += "</body></html>";
      
      server.send(200, "text/html", response);
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  server.begin();
}

void loop() {
  server.handleClient();
  
  unsigned long currentMillis = millis();

  if (doorIsOpening && (currentMillis - doorTriggerTime >= OPEN_DURATION)) {
    digitalWrite(RELAY_PIN, LOW);
    doorIsOpening = false;
  }

  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousWifiMillis >= WIFI_CHECK_INTERVAL)) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousWifiMillis = currentMillis;
  }
}