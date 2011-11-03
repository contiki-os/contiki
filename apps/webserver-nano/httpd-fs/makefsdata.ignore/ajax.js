<script>function lo(){li();}window.onload=lo;</script>
<script type="text/javascript">
var sts;
function li(){ls();}
function ls() {
 var r;try{r=new XMLHttpRequest();}catch(e) {alert("Your browser does not support AJAX!");return false;}
 r.open("GET", "/ajaxdata.shtml", true);//r.setRequestHeader("If-Modified-Since","Sat, 1 Jan 2000 00:00:00 GMT");
 var lx=0;sts=new Date();r.onreadystatechange=function() {
  if(r.readyState>0) {var img=document.getElementById("date");img.innerHTML=(new Date()).toTimeString();var lr=r.responseText.substr(lx);lx+=lr.length;eval(lr);}
  if (ls.stop) r.abort();
 }
 r.send(null);
}
function e(el){d=document;if(d.getElementById){return d.getElementById(el);}else if (d.all){return d.all[el];}}
function s(el,n,max,text){e(el).innerHTML = '<table width=504 border=0 cellpadding=1 cellspacing=0>'+'<tr><td width=200>'+text+'</td>'+'<td width='+(10+300*n/max)+' bgcolor="gray">&nbsp;</td>'+'<td width='+(310-300*n/max)+' bgcolor="lightgray">&nbsp;</td>'+'</table>';}
function dc(n,d){return n.toFixed(d);}
function t(m){n=dc(m/10,1);s('temp',n,50,'Temperature '+n+' &deg;C');}
function b(m){s('batt',m,5000,'Battery '+dc(m/1000,3)+' volts');}
function ax(m){s('ax',m+2000,4000,'Acceleration (X-axis) '+m+'mg');}
function ay(m){s('ay',m+2000,4000,'Acceleration (Y-axis) '+m+'mg');}
function az(m){s('az',m+2000,4000,'Acceleration (Z-axis) '+m+'mg');}
function rs(m){s('rs',n,100,'RSSI '+n);}
function p(c,l,t,r){
  tm=c+1;cp=c*18/tm;lt=t*50.4/tm;lr=r*45.6/tm;n=cp+lt+lr;
  s('p',n,100,'Power consumption '+dc(n,2)+' mW');
  s('pc',cp,100,'CPU power '+dc(cp,2)+' mW');
  s('pr',lr,100,'Radio RX power '+dc(lr,2)+' mW');
  s('pt',lt,100,'Radio TX power '+dc(lt,2)+' mW');
}
function adc(a0,a1,a2,a3,a4,a5,a6,a7){
 var el=e('csv'),sv=el.scrollTop;
 el.innerHTML+=dc(((new Date()).getTime()-sts.getTime())/1000,1)+','+a0+','+a1+','+a2+','+a3+','+a4+','+a5+','+a6+','+a7+'\r';
 if((sv+el.clientHeight+30)<el.scrollHeight)el.scrollTop=sv;else el.scrollTop=sv+42;
}
function rime(tx,rx,tl,rl){e('rims').innerHTML='<em>RIMESTATS:</em> Tx= '+tx+'  Rx= '+rx+'  TxL= '+tl+'  RxL= '+rl;}
function ener(cp,tx,rx) {e('ener').innerHTML='<em>ENERGEST :</em> CPU= '+dc(cp/100,2)+'%  Tx= '+dc(tx/100,2)+'%  Rx= '+dc(rx/100,2)+'%';}
function v(n){e('v').innerHTML='Iterations = '+n;}
</script>