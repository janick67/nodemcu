const mqtt = require('mqtt');
const mysql = require('mysql');
const client  = mqtt.connect('mqtt://192.168.1.100',{
    clientId: 'nodejs',
    clean: false,
    reconnectPeriod: 1000 * 1,
    retain:true,
    qos:0});


let bright = 5, // 5 poziomow jasnosci
    red = 0,
    green = 0,
    blue = 0;

client.on('connect', function () {
    console.log('connected to the server');
   client.subscribe('#', { qos: 0 });
});

client.on("error", function(error) {
   console.log("ERROR: ", error);
});

client.on('offline', function() {
    console.log("offline");
});

client.on('reconnect', function() {
    console.log("reconnect");
});


client.on('message', function (topic, message) {
  zapisz_log(topic, message);
  if (topic === '/irda/ir' && message.slice(0,3)+"" ==='167'){
    odczytaj_z_bazy(message);
  }
//wpis_do_bazy(message);
//odczytaj_z_bazy(message);
 //console.log(topic.toString(),message.toString())
})

const db = mysql.createConnection({
  host    : 'localhost',
  user    : 'janick67',
  password: 'janick67a',
  database: 'smartdom'
})

function zapisz_log(topic,message){
  let device = topic.substring(1,topic.slice(1).indexOf('/')+1);
  topic = topic.slice(topic.slice(1).indexOf('/')+2);
  console.log(`device: ${device}, topic: ${topic}, message: ${message}`);
  const body = {"device":device,"topic":topic,"message":message};
  console.log(body);
  const sql = 'INSERT INTO mqttlog SET ?';
  const query = db.query(sql,body, (err, result) => {
    if (err) {
      console.error(err);
    }
      //console.log("wpisano: "+kod);
  })
}

db.connect((err) => {
  if(err) throw err;
  console.log('MySql Connected...');
  zapisz_log("/PI/START","1");
});

function wpis_do_bazy(kod)
{
const body = {"kod":kod};
const sql = 'INSERT INTO kodyIR SET ?';
const query = db.query(sql,body, (err, result) => {
  if (err) {
    console.error(err);
  }
    console.log("wpisano: "+kod);
})
}

function odczytaj_z_bazy(kod)
{
  if (kod == -1) return;
  sql = `Select nazwa from kodyir where kod = ${kod}`;
  console.log('sql = '+ sql);
  const query = db.query(sql, function (err, result) {
    if (err){console.error(err);};
    console.log(result[0].nazwa);
    obsluz_funkcje(result[0].nazwa);
  });
}

const func = {
  on: () => client.publish('/lamp/ster/lampa','1',{ qos: 0 }),
  off: () => client.publish('/lamp/ster/lampa','0',{ qos: 0 }),
  up: () => {if (bright < 5) bright++; odswiez_kolor()},
  down: () => {if (bright > 0) bright--; odswiez_kolor()},
  r: p => {red = 255; green = p*63.75; blue = 0; odswiez_kolor()},
  g: p => {red = 0; green = 255; blue = p*63.75; odswiez_kolor()},
  b: p => {red = p*63.75; green = 0; blue = 255; odswiez_kolor()},
  f0: () => {red = 100; green = 150; blue = 255; odswiez_kolor()},
  f1: () => {},
  f2: () => {},
  f3: () => {},
  f4: () => {red = 0; green = 0; blue = 0; odswiez_kolor()}
}

function odswiez_kolor()
{
  client.publish('/lamp/led/color','0x'+rgb2hex(Math.round(red*(bright/5)),Math.round(green*(bright/5)),Math.round(blue*(bright/5))));
}

function rgb2hex(r,g,b) {
  const red = int2hex(r);
  const green = int2hex(g);
  const blue = int2hex(b);
  return red+green+blue;
};

function int2hex (int) {
  let hex = Number(int).toString(16);
  if (hex.length < 2) hex = "0" + hex;
  return hex;
};

function obsluz_funkcje(name){
  if (typeof name === 'undefined' || name.length < 2) return
  const inne = ['up','down','off','on','f0','f1','f2','f3','f4'];
  try{
  if (inne.indexOf(name) > -1) return func[name]();
  func[name[0]](name[1]);
}catch{}
}
