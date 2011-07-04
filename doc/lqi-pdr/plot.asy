import graph;
size(350,250,IgnoreAspect);

file fin=input("./1000pkt-64len.csv");
fin.csv();
real[][] A=fin.dimension(0,2);
real[][] pdr=transpose(A);

int[] lqi = sequence(100);

int f (int lqi)
{
	if(lqi <= 6) {
		return (int)((real)lqi*.8);
	} else if(lqi <= 11) {
		return (lqi-6)*18;
	} else if (lqi <= 30) {
		return (int)((real)(lqi-11) * 0.5 + 90);
	} else {
		return 100;
	}
}

int[] f_lqi = map(f,lqi);

draw(graph(pdr[1],pdr[0]/10));
draw(graph(lqi,f_lqi), red);

ylimits(0,100);
xlimits(0,100);
xaxis("\rm LQI",BottomTop,LeftTicks("$%.1f$",10,begin=false,end=false,extend=true,pTick=dotted));
yaxis("\rm Packet Deliver Ratio (\%)",LeftRight,RightTicks("$%#.1f$",8,begin=false,end=false,extend=true,pTick=dotted, ptick=dotted));
