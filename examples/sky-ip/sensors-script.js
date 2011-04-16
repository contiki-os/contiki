/* This is the uncompressed version of the script in httpd-fs/sensordata.shtml */
var start, d = document;

i = new Image(50,60);
i.src = "spin.gif";

function load() {
  var img = d.getElementById("spin");
  img.innerHTML = '&nbsp;';
  loadData();
}

function loadData() {
  var r;
  try { r = new XMLHttpRequest(); }
  catch(e) {
    try { r = new ActiveXObject("Msxml2.XMLHTTP"); }
    catch(e) {
      try { r = new ActiveXObject("Microsoft.XMLHTTP"); }
      catch(e) {
        alert("Your browser does not support AJAX!");
        return false;
      }
    }
  }
  r.onreadystatechange = function() {
    if(r.readyState == 1) {
      var img = d.getElementById("spin");
      img.innerHTML = '<img src="spin.gif">';
    }
    if(r.readyState == 4) {
      var img = d.getElementById("spin");
      img.innerHTML = 'took '  +
                      ((new Date()).getTime() -  start.getTime()) / 1000 +
                      ' seconds';
      eval(r.responseText);
     }
  };
  start = new Date();
  r.open("GET", "/sensordata.shtml", true);
  r.send(null);
}

function e(el) {
  if(d.getElementById) {
    return d.getElementById(el);
  }
  if (d.all) {
    return d.all[el];
  }
}
function s(el,n,max,text) {
  e(el).innerHTML = '<table width=504 border=0 cellpadding=1 cellspacing=0>'+
                    '<tr><td width=200>' +
                    text + '</td>' +
		    '<td width=' + (300*n/max) + ' bgcolor="gray">&nbsp;</td>' +
		    '<td width=' + (300-300*n/max) + ' bgcolor="lightgray">&nbsp;</td>' +
		    '</table>';
}
function dc(n,d) {
  return n.toFixed(d);
}
function t(m) {
  n = dc(-39.6+0.01*m, 1);
  s('temp',n,40,'Temperature '+n+' &deg;C');
}
function h(m) {
  n = dc(-4+0.0405*m - 2.8e-6*(m*m), 2);
  s('hum',n,100,'Humidity '+n+'%');
}
function l1(m) {
  n = dc(.7629394375*m, 0);
  s('l1',n,200,'Light 1 '+n);
}
function l2(m) {
  n = dc(.4693603*m, 0);
  s('l2',n,200,'Light 2 '+n);
}
function rs(m) {
  n = m + 45;
  s('rs',n,100,'RSSI '+n);
}
function p(c,l,t,r) {
  tm=c+l;
  cp=c*1.8/tm;
  lp=l*0.0545/tm;
  lt=t*17.7/tm;
  lr=r*20/tm;
  n=cp+lp+lt+lr;
  s('p',n,30,'Power consumption '+dc(n,2)+' mW');
  s('pc',cp,30,'CPU power '+dc(cp,2)+' mW');
  s('pl',lp,30,'LPM power '+dc(lp,2)+' mW');
  s('pr',lr,30,'Radio RX power '+dc(lr,2)+' mW');
  s('pt',lt,30,'Radio TX power '+dc(lt,2)+' mW');
}
function i(n) {
  e('i').innerHTML = n;
}
