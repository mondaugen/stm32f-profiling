function [] = plot_raw_sound(path,format="int16",channels=2,channel=1)
f=fopen(path,"r");
x=fread(f,Inf,format);
x=x(channel:channels:end);
plot(0:(length(x)-1),x);
