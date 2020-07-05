var ws;

ws = new WebSocket("ws://"+location.host+"/ws");

/*$("#send").click(function(){
    var ssid = $('#ssid').val();
    var pass = $('#password').val();
    var ip = $('#ip').val();

    ws.send("ssid:" + ssid);
    ws.send("pass:" + pass);
    ws.send("ip:" + ip);

    console.log("ssid:" + ssid);
    console.log("pass:" + pass);
    console.log("ip:" + ip);
});*/

function send(){
    document.getElementById("ssid").value;
    var ssid = document.getElementById("ssid").value;
    var pass = document.getElementById("password").value;
    var ip = document.getElementById("ip").value;

    ws.send(ssid + ":" + pass + ":" + ip);

    ws.close();

    console.log("ssid:" + ssid);
    console.log("pass:" + pass);
    console.log("ip:" + ip);
}