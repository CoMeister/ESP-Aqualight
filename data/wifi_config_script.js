var ws = new WebSocket("ws://"+location.host+"/ws");

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

    var e = document.getElementById("ssid");
    var ssid = e.options[e.selectedIndex].text;

    var pass = document.getElementById("password").value;
    var ip = document.getElementById("ip").value;

    ws.send(ssid + ":" + pass + ":" + ip);

    ws.close();

    console.log("ssid:" + ssid);
    console.log("pass:" + pass);
    console.log("ip:" + ip);
}

document.addEventListener("DOMContentLoaded", function(event) { 
    ws.addEventListener('message', function (event) {
        console.log(event.data);
        var ssidList = event.data.split(';');
        console.log(ssidList);

        for(var i = 0; i < ssidList.length-1; i++){
            var select = document.getElementById("ssid");
            var option = document.createElement("option");
            option.text = ssidList[i];
            select.add(option);
        }
    });
});