//var Chart = require('chart.js');
var ctx = document.getElementById('lightGraph');
var color = Chart.helpers.color;
var myChart;
var ws;
var wsDatas = [];
var ipServ = "192.168.1.6";


var dragOptions = {
    animationDuration: 1000
};
//TODO: mettre des limite au drag et au zoom(en X)
var config = {
    type: 'line',
    data: {
        datasets: [{
            label: 'Light 0',
            backgroundColor: 'rgba(0, 59, 142, 0.7)',    //couleur des points
            borderColor: 'rgba(0, 59, 142, 0.7)',        //couleur trait et bord point
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
            backgroundColor: 'rgba(28, 142, 0, 0.7)',    //couleur des points
            borderColor: 'rgba(28, 142, 0, 0.7)',        //couleur trait et bord point
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
        },{
            label: 'Now',
            backgroundColor: 'rgba(130, 59, 100, 0.3)',    //couleur des points
            borderColor: 'rgba(130, 59, 100, 0.3)',        //couleur trait et bord point
            lineTension: 0,
            fill: false,
            data: [{
                x: new Date('2020-03-25T12:00:00'),     //Valeur dynamiques depuis le c++ -> ok
                y: 0
            }, {
                x: new Date('2020-03-25T12:00:00'),
                y: 100
            }],
        }]
    },options: {
        tooltips:{
            mode: 'nearest'
        },
        maintainAspectRatio: false,
        dragData: true,
        dragX: true,
        dragDataRound: 0,
        dragOptions: {
            // magnet: { // enable to stop dragging after a certain value
            //   to: Math.round
            // },
            showTooltip: true // Recommended. This will show the tooltip while the user 
            // drags the datapoint
          },
          onDragStart: function (e, element) {
            stopZoom();
          },
          onDrag: function (e, datasetIndex, index, value) {
            // change cursor style to grabbing during drag action
            try {
                e.target.style.cursor = 'grabbing'

                //limit between different points
                if(myChart.data.datasets[datasetIndex].data[index-1] != null && new Date(value.x) < new Date(myChart.data.datasets[datasetIndex].data[index-1].x)){
                    value.x = myChart.data.datasets[datasetIndex].data[index-1].x;
                }else if(myChart.data.datasets[datasetIndex].data[index+1] != null && new Date(value.x) > new Date(myChart.data.datasets[datasetIndex].data[index+1].x)){
                    value.x = myChart.data.datasets[datasetIndex].data[index+1].x;
                }
            }catch(error) {
            }
              
            // where e = event
          },
          onDragEnd: function (e, datasetIndex, index, value) {
            // restore default cursor style upon drag release
            try {
                e.target.style.cursor = 'default'
            }catch(error) {
            }
            startZoom();
            // where e = event
          },
          hover: {
            onHover: function(e) {
              // indicate that a datapoint is draggable by showing the 'grab' cursor when hovered
              const point = this.getElementAtEvent(e)
              if (point.length) e.target.style.cursor = 'grab'
              else e.target.style.cursor = 'default'
            }
          },
        responsive: true,
        title: {
            display: true,
            text: 'Percent of light'
        },
        scales: {
            xAxes: [{
                type: 'time',
                time:{
                    //parser: "h:mm:ss",
                    displayFormats: { minute: 'HH:mm' , second: 'HH:mm:ss'},
                    unit: 'minute',
                    stepSize:1
                },
                display: true,
                scaleLabel: {
                    display: true,
                    labelString: 'Date'
                },
                ticks: {
                    min: new Date('2020-03-25T00:00:00'),
                    max: new Date('2020-03-25T23:59:59'),
                    major: {
                        fontStyle: 'bold',
                        fontColor: '#FF0000'
                    }
                }
            }],
            yAxes: [{
                display: true,
                scaleLabel: {
                    display: true,
                    labelString: 'value'
                },
                ticks: {
                    max: 100,
                    min: 0,
                    stepSize: 10
                }
            }]
        },
        plugins: {
            zoom: {
                // Container for pan options
                pan: {
                    // Boolean to enable panning
                    enabled: true,
                    rangeMin: {
                        // Format of min pan range depends on scale type
                        x: new Date('2020-03-25T00:00:00'),
                        y: 0
                    },
                    rangeMax: {
                        // Format of max pan range depends on scale type
                        x: new Date('2020-03-25T23:59:59'),
                        y: 100
                    },
    
                    // Panning directions. Remove the appropriate direction to disable 
                    // Eg. 'y' would only allow panning in the y direction
                    mode: 'xy'
                },
    
                // Container for zoom options
                zoom: {
                    // Boolean to enable zooming
                    enabled: true,
                    rangeMin: {
                        x: new Date('2020-03-25T00:00:00'),
                        y: 0
                    },
                    rangeMax: {
                        x: new Date('2020-03-25T23:59:59'),
                        y: 100
                    },
    
                    // Zooming directions. Remove the appropriate direction to disable 
                    // Eg. 'y' would only allow zooming in the y direction
                    mode: 'xy',
                }
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
    myChart.update();   //réactiver zoom??
}

function resetZoom(){
    myChart.resetZoom();
}

function getDatasFromChart(id){
    var brut = myChart.data.datasets[id].data;
    var length = brut.length;
    var datas = [];
    for(var i = 0; i < brut.length; i++){
        var date = new Date(brut[i].x);
        var hInSec = date.getHours()*3600 + date.getMinutes()*60 + date.getSeconds();
        datas.push([hInSec,brut[i].y]);
    }
    return datas;
}

$(document).ready(function(){
    ctx.height=300;
    myChart = new Chart(ctx, config);
    ws = new WebSocket("ws://" + ipServ + "/ws");

    ws.addEventListener('message', function (event) {
        //console.log('Message from server ', event.data);
        //console.log('Receive JSON: ');
        receiveJson(event.data);
    });

    $("#lightGraph").dblclick(function(e){
        myChart.resetZoom();
    });

    $("#send").click(function(){
        //console.log(getDatasFromChart(0));
        //console.log(getDatasFromChart(1));

        var jsonLed0 = JSON.stringify(getDatasFromChart(0));
        var jsonLed1 = JSON.stringify(getDatasFromChart(1));

        console.log("JSON: ");
        console.log("0" +jsonLed0);
        console.log("1" +jsonLed1);
        ws.send("0" +jsonLed0);
        ws.send("1" + jsonLed1);
    });

    setInterval(function getData(){
        var xhttp = new XMLHttpRequest();

        xhttp.onreadystatechange = function()
        {
            if(this.readyState == 4 && this.status == 200)
            {
                console.log(this.responseText);
            }
        };
    }, 2000);


});

function receiveJson(wsData){
    var msg = JSON.parse(wsData);
    
    //for(var i = 0; i < 2; i++){
        var x = parseInt(msg["id"]);

        var brut = myChart.data.datasets[x].data;
        for(var j = 0; j < msg["data0"].length; j++){

            var hInSec = parseInt(msg["data0"][j]);
            var h = Math.floor(hInSec/3600);
            var m = Math.floor(hInSec/60 - h*60);
            var s = hInSec - h*3600 - m*60;

            var hString;
            var mString;
            var sString;

            if(h < 10){
                hString = '0' + h.toString();
            }else{
                hString = h.toString();
            }

            if(m < 10){
                mString = '0' + m.toString();
            }else{
                mString = m.toString();
            }

            if(s < 10){
                sString = '0' + s.toString();
            }else{
                sString = s.toString();
            }

            //console.log("Hour -->" + hString + ":" + mString + ":" + sString);
            //console.log("Percent of light -->" + parseInt(msg["data1"][j]));
            //console.log(brut[j]);
            brut[j].x = new Date('2020-03-25T'+ hString + ':' + mString + ':' + sString);
            brut[j].y = parseInt(msg["data1"][j]);
        }
    //}
    myChart.update();
}