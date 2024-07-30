//常量变量集合地
String date="";//日期
String day="";//星期几
bool b = 1;//是否显示
int vcc;//电源电压
String payload;//网页源码
int httpCode;//网页响应码
/*String apiurl = "http://api.k780.com/?app=life.time&appkey=60897&sign&format=json";//时间api*/
String apiurl = "http://api.k780.com/?app=life.time&appkey=";//时间api
//屏幕的引脚定义
#define SDA 2
#define SCL 0
//esp8266服务器的主页文件
String a1 ="<!DOCTYPE html><html><head><meta charset='utf-8'><title>主页</title></head><body>";
String a2 ="<div style='width: 50%;height: auto;float: left;border-style: double;'><font size='1' color='cornflowerblue'>esp8266主页</font><br><font size='1' color='black'>以下为可访问接口</font><br>";
String a3 ="<a href='../'>主页</a><br><a href='../time'>时间接口</a><p>屏幕是否显示</p><a href='../clear'>开/关</a></div></body>";
String a4 ="</html>";
String asum= a1+a2+a3+a4;
//esp8266服务器的时间界面源码
String s1 = "<meta charset='utf-8'><meta http-equiv='refresh' content='1'/><title>时间</title>这里是时间显示区<br>";
String s2 = "北京时间<br>";
String kgsj = "    ";
int io0 = 0;
int io2 = 2;
int hour;
int m;
int i;
int s;
String diqu;
String wendu;
int tq;
String fen_x;

int dfh_m[]{
  0,350,393,441,495,556,624,661
};
int dfh_m2[]{
  0,175,196,221,234,262,294,330
};
int dfh_m3[]{
  0,700,786,882,935,1049,1178,1322
};
int dfh[]{
  dfh_m[5],dfh_m[5],dfh_m[6],dfh_m[2],//5562
  dfh_m[1],dfh_m[1],dfh_m2[6],dfh_m[2],//1162
  dfh_m[5],dfh_m[5],dfh_m[6],dfh_m3[1],dfh_m[6],dfh_m[5],dfh_m[1],dfh_m[1],dfh_m2[6],dfh_m[2]//5561651162
};
int dfh_d[]{
  1,1,1,1+1,
  1,1,1,1+1,
  1,1,1,1,1,1,1,1,1,1+1
};
    int i_key = 0;
