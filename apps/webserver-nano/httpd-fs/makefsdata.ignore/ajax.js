<script>function lo(){ls();}window.onload=lo;</script>
<script type="text/javascript">var sts=new Date(),r,x,b,j;
function e(el){return document.getElementById(el);}
function ls(){try{r=new XMLHttpRequest();}catch(e){alert("Browswer does not support Ajax");return false;}r.open("GET","/ajaxdata.shtml",true);
x=0;r.onreadystatechange=function(){e('date').innerHTML=(new Date()).toTimeString();if(r.readyState>0){j=r.responseText.substr(x);x+=j.length;eval(j);}};r.send(null);}
function tb(x){var t=e('ss');try{r.abort();}if(x||t.value=="RUN"){t.value="STOP";ls();}else t.value="RUN";}
function s(el,n,max,text){e(el).innerHTML='<table width=504 border=0 cellpadding=1 cellspacing=0>'+'<tr><td width=200>'+text+'</td>'+'<td width='+(10+300*n/max)+' bgcolor="gray">&nbsp;</td>'+'<td width='+(310-300*n/max)+' bgcolor="lightgray">&nbsp;</td>'+'</table>';}
function wt(m){document.title=m;e('v').innerHTML=m;}
function dc(n,d){return n.toFixed(d);}
function t(m){n=dc(m/10,1);s('temp',n,50,'Temperature '+n+' &deg;C');}
function b(m){s('batt',m,5000,'Battery '+dc(m/1000,3)+' volts');}
function ax(m){s('ax',m+2000,4000,'Acceleration (X-axis) '+m+'mg');}
function ay(m){s('ay',m+2000,4000,'Acceleration (Y-axis) '+m+'mg');}
function az(m){s('az',m+2000,4000,'Acceleration (Z-axis) '+m+'mg');}
function rs(m){s('rs',n,100,'RSSI '+n);}
function p(c,l,t,r){cp=0.00109*c;lt=0.00848*t;lr=0.00617*r;n=cp+lt+lr;//mc1322x
 s('p',n,200,'Power consumption '+dc(n,2)+' mW');
 s('pc',cp,100,'CPU power '+dc(cp,2)+' mW');
 s('pr',lr,200,'Radio RX power '+dc(lr,2)+' mW');
 s('pt',lt,200,'Radio TX power '+dc(lt,2)+' mW');}
function adc(a0,a1,a2,a3,a4,a5,a6,a7){var t=e('csv'),s=t.scrollTop;
 t.value+=dc(((new Date()).getTime()-sts.getTime())/1000,1)+','+a0+','+a1+','+a2+','+a3+','+a4+','+a5+','+a6+','+a7+'\r';
 if((s+t.clientHeight+30)<t.scrollHeight)t.scrollTop=s;else t.scrollTop=s+42;}
function rime(tx,rx,tl,rl){e('rims').innerHTML='<em>RIMESTATS:</em> Tx= '+tx+'  Rx= '+rx+'  TxL= '+tl+'  RxL= '+rl;}
function ener(cp,tx,rx) {e('ener').innerHTML='<em>ENERGEST :</em> CPU= '+dc(cp/100,2)+'%  Tx= '+dc(tx/100,2)+'%  Rx= '+dc(rx/100,2)+'%';}
function v(n){e('v').innerHTML='Iterations = '+n;}
</script>