/* The source Code from : https://github.com/riyadhasan24
 * By Md. Riyad Hasan
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* apSsid = "Power Note";
const char* apPass = "12345678";

LiquidCrystal_I2C lcd(0x25, 16, 2);
WebServer server(80);

struct StationData 
{
  int mw = 0;
  uint32_t version = 0;     // increments when new MW is set
  bool acked = false;
  uint32_t lastAckMs = 0;
};

StationData st1, st2;

StationData* getStation(int st) 
{
  if (st == 1) return &st1;
  if (st == 2) return &st2;
  return nullptr;
}

String htmlPage() 
{
  String a1 = st1.acked ? "Received" : "Not received";
  String a2 = st2.acked ? "Received" : "Not received";

  String badge1 = st1.acked ? "badge ok" : "badge wait";
  String badge2 = st2.acked ? "badge ok" : "badge wait";

  String page =
  
R"=====(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
  <title>Power_Note</title>
  <style>
    :root{
      --bg:#0b1220;
      --card:#0f1a33;
      --muted:#9fb0d0;
      --text:#eaf1ff;
      --line:rgba(255,255,255,.10);
      --ok:#23c55e;
      --warn:#f59e0b;
      --btn:#2563eb;
      --btn2:#1d4ed8;
      --shadow:0 10px 30px rgba(0,0,0,.35);
      --radius:16px;
    }
    *{box-sizing:border-box}
    body{
      margin:0;
      font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial,sans-serif;
      background: radial-gradient(1200px 600px at 20% 0%, rgba(37,99,235,.25), transparent 55%),
                  radial-gradient(900px 600px at 90% 10%, rgba(35,197,94,.18), transparent 55%),
                  var(--bg);
      color:var(--text);
      padding: 18px;
    }
    .wrap{max-width: 720px; margin:0 auto;}
    .top{
      display:flex; align-items:flex-start; justify-content:space-between; gap:12px;
      margin-bottom: 14px;
    }
    .title h1{font-size:20px; margin:0 0 4px 0; letter-spacing:.2px}
    .title .sub{color:var(--muted); font-size:13px}
    .pill{
      border:1px solid var(--line);
      background: rgba(255,255,255,.04);
      padding:10px 12px;
      border-radius: 999px;
      font-size: 13px;
      color: var(--muted);
      white-space: nowrap;
    }
    .card{
      background: linear-gradient(180deg, rgba(255,255,255,.06), rgba(255,255,255,.03));
      border:1px solid var(--line);
      box-shadow: var(--shadow);
      border-radius: var(--radius);
      padding: 14px;
    }
    .grid{
      display:grid;
      grid-template-columns: 1fr 1fr;
      gap: 12px;
      margin-top: 12px;
    }
    .sCard{
      border:1px solid var(--line);
      background: rgba(0,0,0,.18);
      border-radius: 14px;
      padding: 12px;
      min-height: 122px;
      display:flex;
      flex-direction:column;
      justify-content:space-between;
    }
    .row{
      display:flex; align-items:center; justify-content:space-between; gap:10px;
    }
    .station{
      font-size: 14px;
      color: var(--muted);
      letter-spacing:.2px;
    }
    .badge{
      padding: 6px 10px;
      border-radius: 999px;
      font-size: 12px;
      border:1px solid var(--line);
    }
    .badge.ok{ background: rgba(35,197,94,.14); color: #bff2cf; border-color: rgba(35,197,94,.25); }
    .badge.wait{ background: rgba(245,158,11,.12); color: #ffe4b5; border-color: rgba(245,158,11,.25); }

    .field label{
      display:block;
      font-size: 13px;
      color: var(--muted);
      margin: 10px 0 6px;
    }
    .field input{
      width:100%;
      padding: 12px 12px;
      border-radius: 12px;
      border:1px solid var(--line);
      background: rgba(255,255,255,.05);
      color: var(--text);
      outline: none;
      font-size: 16px;
    }
    .field input:focus{
      border-color: rgba(37,99,235,.55);
      box-shadow: 0 0 0 4px rgba(37,99,235,.20);
    }
    .btn{
      width:100%;
      margin-top: 12px;
      padding: 12px 14px;
      border:none;
      border-radius: 14px;
      font-size: 16px;
      font-weight: 700;
      color:white;
      background: linear-gradient(180deg, var(--btn), var(--btn2));
      cursor:pointer;
    }
    .btn:active{ transform: translateY(1px); }
    .hint{
      margin-top: 10px;
      color: var(--muted);
      font-size: 12px;
      line-height: 1.5;
    }
    .mw{
      font-size: 28px;
      font-weight: 800;
      letter-spacing: .2px;
    }
    .mw small{font-size: 13px; color: var(--muted); font-weight: 600;}
    .meta{
      margin-top: 6px;
      display:flex;
      gap:10px;
      flex-wrap: wrap;
      color: var(--muted);
      font-size: 12px;
    }
    .kv{
      border:1px solid var(--line);
      background: rgba(255,255,255,.04);
      padding: 6px 10px;
      border-radius: 999px;
    }
    @media (max-width: 640px){
      body{ padding: 14px; }
      .top{ flex-direction: column; align-items: stretch; }
      .pill{ width: fit-content; }
      .grid{ grid-template-columns: 1fr; }
      .mw{ font-size: 30px; }
    }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="top">
      <div class="title">
        <h1>Power_Note — Transmitter</h1>
        <div class="sub">Set MW for Station_1 and Station_2 and send instantly.</div>
      </div>
      <div class="pill">AP: <b>Power Note</b> · IP: <b>192.168.4.1</b></div>
    </div>

    <div class="card">
      <form method="GET" action="/set" autocomplete="off">
        <div class="grid">
          <div class="sCard">
            <div class="row">
              <div class="station">Station_1</div>
              <div class="badge1"></div>
            </div>

            <div class="field">
              <label for="mw1">Load (MW)</label>
              <input id="mw1" name="mw1" type="number" min="0" max="99999" inputmode="numeric" value=")=====" + String(st1.mw) + R"=====(" required>
            </div>

            <div>
              <div class="mw">)=====" + String(st1.mw) + R"=====( <small>MW</small></div>
              <div class="meta">
                <div class="kv">Status: )=====" + a1 + R"=====(</div>
                <div class="kv">v)=====" + String(st1.version) + R"=====(</div>
              </div>
            </div>
          </div>

          <div class="sCard">
            <div class="row">
              <div class="station">Station_2</div>
              <div class="badge2"></div>
            </div>

            <div class="field">
              <label for="mw2">Load (MW)</label>
              <input id="mw2" name="mw2" type="number" min="0" max="99999" inputmode="numeric" value=")=====" + String(st2.mw) + R"=====(" required>
            </div>

            <div>
              <div class="mw">)=====" + String(st2.mw) + R"=====( <small>MW</small></div>
              <div class="meta">
                <div class="kv">Status: )=====" + a2 + R"=====(</div>
                <div class="kv">v)=====" + String(st2.version) + R"=====(</div>
              </div>
            </div>
          </div>
        </div>

        <button class="btn" type="submit">Send to Stations</button>

        <div class="hint">
          Tip: Only the station whose MW value changes will trigger a new alarm.
          Receivers acknowledge by long-pressing the button.
        </div>
      </form>
    </div>
  </div>

  <script>
    // inject badges (tiny script keeps HTML string simple)
    (function(){
      const b1 = document.querySelector('.badge1');
      const b2 = document.querySelector('.badge2');
      if (b1) b1.className = ')=====" + badge1 + R"=====(';
      if (b2) b2.className = ')=====" + badge2 + R"=====(';
      if (b1) b1.textContent = ')=====" + a1 + R"=====(';
      if (b2) b2.textContent = ')=====" + a2 + R"=====(';
    })();
  </script>
</body>
</html>)=====";

  return page;
}


void showLcdStatus() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("S1:");
  lcd.print(st1.mw);
  lcd.print("MW ");
  lcd.print(st1.acked ? "OK" : "--");

  lcd.setCursor(0, 1);
  lcd.print("S2:");
  lcd.print(st2.mw);
  lcd.print("MW ");
  lcd.print(st2.acked ? "OK" : "--");
}

// Home UI
void handleRoot() 
{
  server.send(200, "text/html", htmlPage());
}

void handleSet() 
{
  if (!server.hasArg("mw1") || !server.hasArg("mw2")) 
  {
    server.send(400, "text/plain", "Missing mw1 or mw2");
    return;
  }

  int mw1 = server.arg("mw1").toInt();
  int mw2 = server.arg("mw2").toInt();

  if (mw1 != st1.mw) 
  {
    st1.mw = mw1;
    st1.version++;
    st1.acked = false;
  }

  if (mw2 != st2.mw) 
  {
    st2.mw = mw2;
    st2.version++;
    st2.acked = false;
  }

  showLcdStatus();

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Sent Successfully");
}

void handleGet() 
{
  if (!server.hasArg("st")) 
  {
    server.send(400, "application/json", "{\"err\":\"missing st\"}");
    return;
  }
  int st = server.arg("st").toInt();
  StationData* s = getStation(st);
  if (!s) 
  {
    server.send(400, "application/json", "{\"err\":\"invalid st\"}");
    return;
  }

  String json = "{";
  json += "\"st\":" + String(st) + ",";
  json += "\"mw\":" + String(s->mw) + ",";
  json += "\"v\":" + String(s->version) + ",";
  json += "\"acked\":" + String(s->acked ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

void handleAck() 
{
  if (!server.hasArg("st")) 
  {
    server.send(400, "application/json", "{\"err\":\"missing st\"}");
    return;
  }
  int st = server.arg("st").toInt();
  StationData* s = getStation(st);
  if (!s) 
  {
    server.send(400, "application/json", "{\"err\":\"invalid st\"}");
    return;
  }

  s->acked = true;
  s->lastAckMs = millis();
  showLcdStatus();

  String msg = "{\"ok\":true,\"msg\":\"Station-" + String(st) + " Received\"}";
  server.send(200, "application/json", msg);
}

void setup() 
{
  Serial.begin(115200);

  Wire.begin();
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Power_Note");
  lcd.setCursor(0, 1);
  lcd.print("Starting AP...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPass);

  delay(300);
  IPAddress ip = WiFi.softAPIP(); // usually 192.168.4.1

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AP: Power Note");
  lcd.setCursor(0, 1);
  lcd.print(ip.toString());

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/get", handleGet);
  server.on("/ack", handleAck);

  server.begin();

  delay(800);
  showLcdStatus();
}

void loop() 
{
  server.handleClient();
}
