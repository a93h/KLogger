(function() {
  'use strict';

  function $(id) {
    return document.getElementById(id);
  };

  function repaintAll() {
    var data = [];
    var max = 0;
    var temp = {};
    var currentText = typeField.value;
    var currentLength = currentText.length;
    var coordinates = app.coordinates;
    var klogArr;
    if ($('klogger').checked) {
      var klogArr = currentText.split(',')
      klogArr.forEach(vk => {
        var k = parseInt(vk)
        console.log('virtual key processing: dec: ' + k.toString(10) + ' hex: ' + k.toString(16))
        var coord
        coord = coordinates[app.KLOGGER[k]] || false
        if (coord){
          for(var s = 0; s < coord.length; s += 2) {
            var joined = coord.slice(s, s+2).join(";");
            if(!temp[joined])
              temp[joined] = 0;

            temp[joined] += 1;
          }
        } else {
          console.log('virtual key not found in common keys: dec: ' + k.toString(10) + ' hex: ' + k.toString(16))
          coord = app.EXTRAS[k]
          if (coord){
            for(var s = 0; s < coord.length; s += 2) {
              var joined = coord.slice(s, s+2).join(";");
              if(!temp[joined])
                temp[joined] = 0;

              temp[joined] += 1;
            }
          } else {
            console.log('virtual key not found in control keys: dec: ' + k.toString(10) + ' hex: ' + k.toString(16))
          }
        }
      })
    } else {
      for(var i=0; i < currentLength; i++){

        var key = currentText.charAt(i);
        if(/^[A-Za-z]$/.test(key)){
          key = key.toUpperCase();
        }
        if(app.config.exclude && app.EXCLUDES.indexOf(key) == -1){
          var coord;
          coord = coordinates[key] || false;
          if(coord){
            for(var s = 0; s < coord.length; s += 2) {
              var joined = coord.slice(s, s+2).join(";");
              if(!temp[joined])
                temp[joined] = 0;

              temp[joined] += 1;
            }
          }
        }
      }
    }

    for(var k in temp){
      var xy = k.split(";");
      var val = temp[k];

      data.push({x: xy[0], y: xy[1], count: val});

      if(val > max)
        max = val;
    }

    app.heatmap.store.setDataSet({max: max, data: data});
  };

  var typeField = $('typefield');
  var currentTypeFieldLen = typeField.value.length;


  app.init = function initialize() {
    var cfg = arguments[0] || {};
    app.configure(cfg);
    repaintAll();
  };

  app.configure = function configure(cfg) {
    var config = {};
    config.element = "keyboard";
    config.radius = cfg.radius || 50;
    config.visible = true;
    config.opacity = 40;
    if(cfg.gradient)
      config.gradient = cfg.gradient;

    app.coordinates = app.LAYOUTS[cfg.layout || "QWERTY"];

    var heatmap = h337.create(config);
    app.heatmap = heatmap;
    if(cfg.layout)
      $("keyboard").style.backgroundImage = "url(img/"+cfg.layout+".png)";
  };


  window.onload = app.init;

  var gradients = [
    {  0.0001: "rgb(16, 0, 255)", 0.001: "rgb(117, 0, 244)", 0.01: "rgb(162, 0, 232)", 0.025: "rgb(196, 0, 218)", 0.05: "rgb(224, 0, 203)",
		0.10: "rgb(247, 0, 187)", 0.15: "rgb(255, 0, 171)", 0.20: "rgb(255, 0, 155)", 0.25: "rgb(255, 0, 140)", 0.30: "rgb(255, 0, 124)",
		0.35: "rgb(255, 0, 109)", 0.40: "rgb(255, 0, 95)", 0.45: "rgb(255, 0, 80)", 0.50: "rgb(255, 0, 66)", 0.55: "rgb(255, 65, 50)",
		0.60: "rgb(255, 96, 32)", 0.65: "rgb(255, 120, 0)", 0.70: "rgb(255, 141, 0)", 0.75: "rgb(255, 161, 0)", 0.80: "rgb(255, 178, 0)",
		0.85: "rgb(255, 195, 0)", 0.90: "rgb(255, 211, 0)", 0.95: "rgb(255, 226, 0)", 0.98: "rgb(255, 240, 0)", 1.0: "rgb(255, 254, 0)"}, // standard
    { 0.0001: "rgb(255, 255, 255)", 0.001: "rgb(239, 255, 254)", 0.01: "rgb(223, 255, 253)", 0.025: "rgb(205, 255, 252)", 0.05: "rgb(187, 255, 251)",
		0.10: "rgb(167, 255, 250)", 0.15: "rgb(144, 255, 249)", 0.20: "rgb(118, 255, 248)", 0.25: "rgb(84, 255, 247)", 0.30: "rgb(2, 255, 246)",
		0.35: "rgb(2, 255, 246)", 0.40: "rgb(0, 241, 238)", 0.45: "rgb(0, 227, 230)", 0.50: "rgb(0, 213, 221)", 0.55: "rgb(0, 199, 212)",
		0.60: "rgb(0, 185, 201)", 0.65: "rgb(9, 172, 190)", 0.70: "rgb(23, 158, 179)", 0.75: "rgb(31, 145, 167)", 0.80: "rgb(37, 132, 155)",
		0.85: "rgb(41, 120, 142)", 0.90: "rgb(43, 107, 129)", 0.95: "rgb(44, 95, 115)", 0.98: "rgb(43, 83, 102)", 1.0: "rgb(42, 72, 88)"}, // nightly
    { 0.0001: "rgb(0,220,255)", 0.001: "rgb(0,216,255)", 0.01: "rgb(0,212,255)", 0.025: "rgb(0, 207, 255)", 0.05: "rgb(0, 202, 255)",
		0.10: "rgb(0, 197, 255)", 0.15: "rgb(0, 191, 255)", 0.20: "rgb(0, 185, 255)", 0.25: "rgb(41, 178, 255)", 0.30: "rgb(78, 170, 255)",
		0.35: "rgb(106, 162, 255)", 0.40: "rgb(130, 153, 255)", 0.45: "rgb(152, 143, 255)", 0.50: "rgb(171, 132, 255)", 0.55: "rgb(190, 119, 255)",
		0.60: "rgb(206, 106, 249)", 0.65: "rgb(221, 90, 235)", 0.70: "rgb(235, 72, 219)", 0.75: "rgb(246, 49, 202)", 0.80: "rgb(255, 0, 184)",
		0.85: "rgb(255, 0, 184)", 0.90: "rgb(255, 47, 129)", 0.95: "rgb(255, 125, 71)", 0.98: "rgb(255, 194, 0)", 1.0: "rgb(255, 254, 0)" } // fanzy
  ];
  var lastValue = "";

  typeField.oninput = function() {
    var currentValue = this.value;
    if (Math.abs(lastValue.length - currentValue.length) >= 1) {
      repaintAll();
    } else {
      var key = (currentValue.length > lastValue.length) ? (currentValue.split(lastValue)[1]) : (lastValue.split(currentValue)[1]);

      if(/^[A-Za-z]$/.test(key)){
        key = key.toUpperCase();
      }
      if(app.config.exclude && app.EXCLUDES.indexOf(key) == -1){
        var coord = app.coordinates[key]
        if (coord) {
          for (var s = 0; s < coord.length; s+=2) {
            app.heatmap.store.addDataPoint.apply(app.heatmap.store,coord.slice(s, s+2));
          }
        }
      }
    }

    lastValue = currentValue;
  };

  $("layout").onchange = $("fingertip").onchange = $("gradient").onchange = function(){
    var cfg = {};
    cfg.radius = Math.pow(10,$("fingertip").selectedIndex) +40;
    cfg.gradient = gradients[$("gradient").selectedIndex];
    cfg.layout =  $("layout").value;
    app.heatmap.cleanup();
    app.init(cfg);
    repaintAll();
  }

  var items = document.getElementsByTagName("li");
  for(var i=0; i < items.length; i++){
    (function(i){
      items[i].onclick = function(){
        typeField.value = app.SAMPLE_TEXT[i];
        repaintAll();
      };
    })(i);
  }

})();