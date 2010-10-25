import graph;
size(350,250,IgnoreAspect);

file fin=input("./1000pkt-64len.csv");
real[][] A=dimension(csv(fin),0,2);
real[][] pdr=transpose(A);

draw(graph(pdr[1],pdr[0]));

ylimits(0,1000);
xlimits(0,100);
xaxis("\rm Output Current (A)",BottomTop,LeftTicks("$%.1f$",10,begin=false,end=false,extend=true,pTick=dotted));
yaxis("\rm Output Power (W)",LeftRight,RightTicks("$%#.1f$",8,begin=false,end=false,extend=true,pTick=dotted, ptick=dotted));
