//var Chart = require('chart.js');
/*import Chart from 'chart.js'
// load the options file externally for better readability of the component.
// In the chartOptions object, make sure to add "dragData: true" etc.
import chartOptions from '~/assets/js/labour.js'
import 'chartjs-plugin-dragdata'*/

var ctx = document.getElementById('lightGraph');
var color = Chart.helpers.color;
var myChart;
var ws;
var wsDatas = [];
var ipServ = "0.0.0.0";
var wconfIsOpen = false;

var maxRange = 100;


var dragOptions = {
    animationDuration: 750
};

var config = {
    type: 'line',
    data: {
        datasets: [{
            label: 'Light 0',
            backgroundColor: 'rgba(153, 183, 151, 0.7)',    //round color blue
            borderColor: 'rgba(153, 183, 151, 0.7)',        //Line and round border color
            lineTension: 0,
            fill: false,
            data: [{
                x: new Date('2020-03-25T12:00:00'),     //Valeur dynamiques depuis le c++ -> ok
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }],
        }, {
            label: 'Light 1',
            backgroundColor: 'rgba(169, 24, 92, 0.5)',    //round color yellow
            borderColor: 'rgba(169, 24, 92, 0.5)',        //Line and round border color
            lineTension: 0,
            fill: false,
            data: [{
                x: new Date('2020-03-25T12:00:00'),     //Valeur dynamiques depuis le c++
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 50
            }]
        }, {
            label: '00:00:00',
            backgroundColor: 'rgba(30, 30, 30, 0.4)',    //round color pink
            borderColor: 'rgba(30, 30, 30, 0.4)',        //Line and round border color
            lineTension: 0,
            fill: false,
            dragData: false,
            data: [{
                x: new Date('2020-03-25T12:00:00'),     //Valeur dynamiques depuis le c++ -> ok
                y: 0
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: maxRange
            }],
        }]
    },
    options: {
        /*interaction: {
            mode: 'nearest'
        },
        onHover: function (e) {
            // indicate that a datapoint is draggable by showing the 'grab' cursor when hovered
            //const point = this.getElementAtEvent(e);
            //if (point.length) e.target.style.cursor = 'grab';
            //else e.target.style.cursor = 'default';
            //e.target.style.cursor = 'grab';
            console.log(e);
            
        },*/
        maintainAspectRatio: false,
        title: {
            display: true,
            text: 'Percent of light'
        },
        scales: {
            x: {
                type: 'time',
                time: {
                    //parser: "h:mm:ss",
                    displayFormats: { minute: 'HH:mm', second: 'HH:mm:ss' },
                    unit: 'minute',
                    stepSize: 30
                },
                min: new Date('2020-03-25T00:00:00'),
                max: new Date('2020-03-25T23:59:59')
            },
            y: {
                max: maxRange,
                min: 0,
                stepSize: 10
            }
        },
        plugins: {
            dragData: {
                dragX: true,
                round: 0,
                showTooltip: true,
                onDragStart: function (e, datasetIndex, index, value) {
                    stopZoom();
                },
                onDrag: function (e, datasetIndex, index, value) {
                    e.target.style.cursor = 'grabbing';
                    var actualPointDate = new Date(value.x);

                    //limit between different points
                    if (myChart.data.datasets[datasetIndex].data[index - 1] != null && actualPointDate < new Date(myChart.data.datasets[datasetIndex].data[index - 1].x)) {
                        value.x = myChart.data.datasets[datasetIndex].data[index - 1].x+1;
                    } else if (myChart.data.datasets[datasetIndex].data[index + 1] != null && actualPointDate > new Date(myChart.data.datasets[datasetIndex].data[index + 1].x)) {
                        value.x = myChart.data.datasets[datasetIndex].data[index + 1].x-1;
                    }else{
                        $("#pointInfo").text("Amount: " + value.y + "% At: " + actualPointDate.getHours().toString().padStart(2, '0') + ":" + actualPointDate.getMinutes().toString().padStart(2, '0') + ":" + actualPointDate.getSeconds().toString().padStart(2, '0'));
                    }

                },
                onDragEnd: function (e, datasetIndex, index, value) {
                    e.target.style.cursor = 'default';
                    startZoom();
                    $("#pointInfo").text("Amount: 0% At: 00:00:00");
                }
            },
            //responsive: true,
            zoom: {
                // Container for pan options
                pan: {
                    enabled: true,
                    mode: 'xy'
                },
                zoom: {
                    enabled: true,
                    mode: 'xy'
                },
                limits: {
                    x: {
                        min: new Date('2020-03-25T00:00:00'),
                        max: new Date('2020-03-25T23:59:59')
                    },
                    y: {
                        min: 0,
                        max: 100
                    }
                },
                tooltip: {
                    backgroundColor: 'rgba(255, 51, 0, 0.60)'
                },
            }

        }
    }
};

function stopZoom() {
    //console.log("Stop zoom");
    myChart.options.plugins.zoom.pan.enabled = false;
    myChart.options.plugins.zoom.zoom.enabled = false;
}

function startZoom() {
    //console.log("start zoom");
    myChart.options.plugins.zoom.pan.enabled = true;
    myChart.options.plugins.zoom.zoom.enabled = true;
    myChart.update();   //reactivate zoom??
}

function resetZoom() {
    myChart.resetZoom();
}

function getDatasFromChart(id) {
    var brut = myChart.data.datasets[id].data;
    var length = brut.length;
    var datas = [];
    for (var i = 0; i < brut.length; i++) {
        var date = new Date(brut[i].x);
        var hInSec = date.getHours() * 3600 + date.getMinutes() * 60 + date.getSeconds();
        datas.push([hInSec, brut[i].y]);
    }
    return datas;
}

$(document).ready(function () {
    ipServ = location.host;
    ipServ = "192.168.1.6";
    ctx.height = 300;
    myChart = new Chart(ctx, config);
    ws = new WebSocket("ws://" + ipServ + "/ws");

    ws.addEventListener('message', function (event) {
        console.log('Message from server=', event.data);
        try {
            receiveJson(event.data);
        } catch (error) {
            var ssidList = event.data.split(';');
            console.log("ssidList: " + ssidList);
            var select = $("#ssid");
            select.find('option').remove();

            for (var i = 0; i < ssidList.length - 1; i++) {
                var option = document.createElement("option");
                option.text = ssidList[i];
                select[0].add(option);
            }
        }
    });

    $("#lightGraph").dblclick(function (e) {
        myChart.resetZoom();
    });

    $("#send").click(function () {
        //console.log(getDatasFromChart(0));
        //console.log(getDatasFromChart(1));

        var jsonLed0 = JSON.stringify(getDatasFromChart(0));
        var jsonLed1 = JSON.stringify(getDatasFromChart(1));

        console.log("JSON: ");
        console.log("0" + jsonLed0);
        console.log("1" + jsonLed1);
        ws.send("0" + jsonLed0);
        ws.send("1" + jsonLed1);
        ws.send(JSON.stringify(this.checked));
    })

    $("#forceLight").click(function () {
        console.log($("#forceLight").prop('checked').toString());
        ws.send($("#forceLight").prop('checked').toString());
    });


    var now = 0;
    $(document).on('input', '#lightLevelRange', function () {
        //console.log(this.value);
        //console.log("lightLevel:" + this.value);

        $("#rangeLab").text("Light level: " + this.value + "%");
        if (($.now() - now) >= 250) {
            now = $.now();
            ws.send("lightLevel:" + this.value);
        }

    });

    /*$("#lightLevelRange").mouseup(function () {   //Doesn't work on mobile :3
        console.log(this.value);
        console.log("lightLevel:" + this.value);
        $("#rangeLab").text("Light level: "+this.value + "%");
        
        ws.send("lightLevel:" + this.value)
    });*/

    $("#netConfig").click(function () {
        if (wconfIsOpen) {
            $("#changeWConf").css("display", "none");
            $("#netConfig").css("background-color", "#073B4C");
            $("#netConfig").text("Net config");
            wconfIsOpen = false;
        } else {//if wconfIsOpen == false
            $("#changeWConf").css("display", "block");
            $("#netConfig").css("background-color", "rgb(239, 71, 111)");
            $("#netConfig").css("width", "137px");
            $("#netConfig").text("Cancel");
            ws.send("ssidList");
            wconfIsOpen = true;
        }
    });
    /*$("#refresh").click(function(){
        ws.send("ssidList");
    });*/

    $("#sub").click(function () {
        $("#changeWConf").css("display", "none");
        $("#netConfig").css("background-color", "#073B4C");
        $("#netConfig").text("Net config");
        wconfIsOpen = false;

        document.getElementById("ssid").value;

        var ssid = $("#ssid option:selected").text()

        var pass = $("#password").val();
        var ip = $("#ip").val();

        ws.send(ssid + ":" + pass + ":" + ip);

        console.log("ssid:" + ssid);
        console.log("pass:" + pass);
        console.log("ip:" + ip);
    });

    $("#deconnect").click(function () {
        console.log("deco");
        ws.send("reset");
        ws.close();
    });


    console.log($("#lightLevelRange"));
    $("#rangeLab").text("Light level: " + $("#lightLevelRange")[0].value + "%");
    updateNow();

});

function receiveJson(wsData) {
    if (wsData.substring(0, wsData.indexOf(":")) == "lightLevel") {
        //var isLightForcedChecked = parseInt(substring(wsData.length-1));
        //console.log("isLightForcedChecked="+substring(wsData.length-1));
        var lightLevel = parseInt(wsData.substring(0, wsData.indexOf(",")).substring(wsData.indexOf(":") + 1));
        console.log("lightLevel=" + lightLevel);
        $("#rangeLab").text("Light level: " + lightLevel + "%");
        $("#lightLevelRange").val(lightLevel);
        var isLightForcedChecked = parseInt(wsData.substring(wsData.length - 1));
        console.log("isLightForcedChecked=" + isLightForcedChecked);
        $("#forceLight")[0].checked = isLightForcedChecked;
    } else {
        var msg = JSON.parse(wsData);

        //for(var i = 0; i < 2; i++){
        var x = parseInt(msg["id"]);

        var brut = myChart.data.datasets[x].data;
        for (var j = 0; j < msg["data0"].length; j++) {

            var hInSec = parseInt(msg["data0"][j]);
            var h = Math.floor(hInSec / 3600);
            var m = Math.floor(hInSec / 60 - h * 60);
            var s = hInSec - h * 3600 - m * 60;

            var hString;
            var mString;
            var sString;

            if (h < 10) {
                hString = '0' + h.toString();
            } else {
                hString = h.toString();
            }

            if (m < 10) {
                mString = '0' + m.toString();
            } else {
                mString = m.toString();
            }

            if (s < 10) {
                sString = '0' + s.toString();
            } else {
                sString = s.toString();
            }

            //console.log("Hour -->" + hString + ":" + mString + ":" + sString);
            //console.log("Percent of light -->" + parseInt(msg["data1"][j]));
            //console.log(brut[j]);
            brut[j].x = new Date('2020-03-25T' + hString + ':' + mString + ':' + sString);
            brut[j].y = parseInt(msg["data1"][j]);
        }
        //}
        myChart.update();
    }
}

function outputUpdate(lightLevelRange) {
    document.querySelector('#volume').value = maxRangeF;
    maxRange = maxRangeF;
    console.log(maxRange);
    myChart.options.plugins.zoom.pan.rangeMax.y = maxRange;
    myChart.options.scales.yAxes.ticks.max = maxRange;
    myChart.options.plugins.zoom.rangeMax.y = maxRange;
}

function checkTime(i) {
    if (i < 10) {
        i = "0" + i;
    }
    return i;
}

function updateNow() {  //
    var today = new Date();
    var h = today.getHours();
    var m = today.getMinutes();
    var s = today.getSeconds();

    //new Date('2020-03-25T12:00:00')

    //new Date('2020-03-25T'+h+':'+m+':'+s)

    h = checkTime(h);
    m = checkTime(m);
    s = checkTime(s);

    time = h + ':' + m + ':' + s;

    myChart.data.datasets[2].data[0].x = new Date('2020-03-25T' + time);
    myChart.data.datasets[2].data[1].x = new Date('2020-03-25T' + time);
    myChart.data.datasets[2].label = time;

    myChart.update('none');


    t = setTimeout(function () {
        updateNow()
    }, 1000);
}