/***************************************************************************************************************************************************
 * 物联网时钟不知道什么版本的源代码
 * @author Moments.
 * @date 2022-01-01 （史前达芬不知道时间了）
 ***************************************************************************************************************************************************/

#include "layout.h"//引入布局头文件
#include "web.h"//引入各个常量变量的头文件
#include <ESP8266WiFi.h>//导入esp8266的WiFi库
#include <ESP8266HTTPClient.h>//http客户端库
#include <ESP8266WebServer.h>//esp8266服务器库
#include <WiFiManager.h>//热点配网库
#include <Adafruit_ssd1306syp.h>//屏幕驱动库
#include <ArduinoJson.h>//json库       
#include <ArduinoOTA.h>
#include <FS.h>
#include <Ticker.h>
#include <DNSServer.h>
#include <DS1302.h>

//时钟芯片管脚定义
#define kCePin   14 // 复位引脚
#define kIoPin   12 // 数据引脚
#define kSclkPin 13 // 时钟引脚
DS1302 rtc(kCePin, kIoPin, kSclkPin);

bool led_open = 0;

Ticker tic;
Adafruit_ssd1306syp display(SDA, SCL); //屏幕驱动的配置
ESP8266WebServer server(80);//服务器端口
//ESP8266WebServer shell_server(8080);//服务器端口
ADC_MODE(ADC_VCC);//设置ADC读取电源电压
DNSServer dns;//dns服务器
const byte dns_port = 53;//dns服务器端口
IPAddress apIP(192, 168, 4, 1);
int length;
void setup() {
  Serial.begin(9600);//打开串口通信
  //初始化ds1302
  rtc.writeProtect(false);
  rtc.halt(false);
  //开机后的一些初始化
  //初始化屏幕
  display.initialize();
  display.clear();//解决花屏
  //然后连接WiFi
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("wait for WiFi");
  display.update();
  int WiFii = 0;//用来计数
  WiFi.hostname("ESP8266 clock");//设置主机名
  WiFiManager wifiManager;
 // wifiManager.resetSettings();//清除WiFi信息
  display.println("Please connect esp8266WiFi");
  display.println("192.168.4.1");
  display.update();
  // 自动连接WiFi
  wifiManager.autoConnect("esp8266");
  display.update();
  vcc = ESP.getVcc();//获取电源电压
  startlayout();
  display.setCursor(0, 28);
  display.println("System is starting...");
  display.update();
 
  server.begin();
  server.onNotFound(NotFound);

  //命令行服务器初始化
//  shell_server.begin();
 // shell_server.onNotFound(shell)
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("esp8266服务器WiFi", "123456789");
  dns.start(dns_port, "www.ydyjk.com", apIP); //配置域名
  SPIFFS.begin();
  gt2();
  gt();
  wd();
}

void loop() {

  display.clear();
  server.handleClient();//服务器服务
 // shell_server.handleClient();
  dns.processNextRequest();//域名解析服务
  if (b) {
    gt();//调用时间函数
    if (hour == 0 && s > 2) {
      gt2();
    }
    toplayout();
    showtime();
    display.update();
    time_clock();
   /*
    * 关机按键设定
    */
    if (digitalRead(0) == 0) {
      i_key++;
      if (i_key == 3) {
        display.clear();
        display.print("");
        display.update();
        display.clear();
        delay(500);
        ESP.deepSleep(0);
      }
    }else i_key = 0;
    delay(1000);
  } else {
    display.clear();
    display.clear();
    display.print("");
    display.update();
    display.clear();
  }

}


/*
   东方红音乐
*/
void dfh_music() {
  for (int i = 0; i < length; i++) {
    tone(2, dfh[i]);
    delay(dfh_d[i] * 600);
    noTone(2);
  }
}

void shell(){
/*	String date_url = shell_server.uri().substring(1);
    String date = NULL;
    String return_str = NULL;
	if(date_url == "set"){
	    if (shell_server.hasArg("date")){
	    	for (int i = 0; i < 30; ++i)
	    	{
	    		date+=shell_server.arg("time").substring(0,i);
	    		if (date == "Time"||date=="time"){
                   shell_server.send(200,"text/plain",date);
                   break;
	    		}else if(date == "pinMode_out"){
	    		   pinMode(date.substring(i,i+3).toInt(),OUTPUT);
	    		   shell_server.send(200,"text/plain","set io ok!");
	    		   break;
	    		}else if(date == "pinMode_in"){
	    		   pinMode(date.substring(i,i+3).toInt(),INPUT);
	    		   shell_server.send(200,"text/plain","set io ok!");
	    		   break;
	    		}else if(date =="pin_set"){
	    			digitalWrite(date.substring(i,i+3)).toInt(), value);
	    		}
	    	}
	    }
	}else{
		shell_server.send(200 , "text/plain","$esp>>");
	}*/
}

void NotFound() {
  String url = server.uri().substring(1);//首先获取请求地址
  if (url == "time") {
    /*
       esp8266服务器的时间界面
    */
    gt();//获取时间
    String s3 = s1 + s2 + date + kgsj + day;//合并网页内容
    server.send(200 , "text/html", s3);
  } else if (url == "sleep") {
    /*
       必须为GET请求
       sleep请求处理
       判断是否有时间限定
       没有或小于1则永久睡眠
    */
    display.clear();
    display.print("");
    display.update();
    display.clear();
    int sleep_method  = server.method();
    if (sleep_method == 1) {
      server.send(200, "text/plain", "ok");
      delay(1000);
      if (server.hasArg("time")) {
        /*
           睡眠时间设定
        */
        int sleep_time = server.arg("time").toInt() * 1000000 ;//装换为秒
        ESP.deepSleep(sleep_time);
      } else {
        ESP.deepSleep(0);
      }
    } else {
      server.send(200, "text/plain", "no");//非GET请求返回
    }
  } else if (url == "rst") {
    /*
      复位请求
    */
    server.send(200, "text/plain", "ok");
    delay(1000);//延迟1s复位
    ESP.restart();
  } else if (url == "oled_clear") {
    /*
       屏幕是否显示
       处理是否显示的布尔值 也就是反转
       然后返回屏幕状态
    */
    b = !b;
    if (b)
      server.send(200, "text/plain", "the oled is open");
    else
      server.send(200, "text/plain", "the oled is close");
  } else if (url == NULL || url == "home" || url == "index.html") {
    /*
       如果url等于空或home则显示主页
    */
    server.send(200, "text/html", asum);//主页内容
  } else if (url == "flash_clear") {
    /*
       清空flash的闪存文件系统
       有卡顿正常
    */
    SPIFFS.format();
    server.send(200, "text/plain", "ok");
  } else if (url == "wd") {
    wd();
    server.send(200, "text/plain", "ok");
  }
  else {
    server.send(404, "text/html", "<h3>404 NOT FOUND</h3>");//主页内容
  }
}


void time_clock()
{
 
}

/*
   开机动画
*/
void startlayout() {
  display.clear();
  display.drawBitmap(0, 0, start, 128, 64, 1);
  display.update();
  delay(1000);
  display.clear();
  display.setCursor(0, 0);
  //显示电源电压
  display.println("Vcc:");
  display.print(vcc);
  display.print("mv");
  display.println();
  //显示WiFi相关信息
  display.println("WiFi ip:");
  display.println(WiFi.localIP());
  display.println("WiFi RSSI:");
  display.println(WiFi.RSSI());
  display.update();
  delay(1000);
  display.clear();
}

/*
   时间界面显示
*/
void toplayout() {
  //横线上面的内容
  display.drawBitmap(0, 0, tq1, 16, 16, 1);
  display.drawBitmap(17, 0, tq2, 16, 16, 1);
  display.setTextSize(1);
  display.setCursor(33, 0);
  display.print(wendu);
  display.drawBitmap(48, 0, du, 8, 8, 1);
  show_tq(tq);
  display.setCursor(0, 18);
  display.println("Day:");
  display.setCursor(25, 18);
  display.print(day);
  display.print("  ");
  display.print(ESP.getVcc() - 200);
  display.print("mv");
  /*
    实现天气的更新
  */
  if (m == 0 || m == 10 || m == 20 || m == 30 || m == 40 || m == 50)
    if (s < 4 && s != 0)
      wd();
}

void showtime() {
  //画一条线
  display.setTextSize(1);
  display.setCursor(0, 23);
  for (int i = 0; i < 21; i++)//用for循环画出来
  {
    display.print("_");
  }
  //显示时间
  display.setCursor(4, 32);
  display.setTextSize(2);
  display.println(date);

}

/*
   时间获取
*/
void gt() {
  if (WiFi.status() == WL_CONNECTED) { //确保WiFi连接
    //免费时间接口
    HTTPClient http;//实例化http客户端对象
    http.begin("http://quan.suning.com/getSysTime.do");//设置请求URL
    //{"sysTime2":"2021-08-12 11:45:16","sysTime1":"20210812114516"}
    http.addHeader("Content-Type", "text/plain");///请求头信息
    httpCode = http.GET();//发送请求 返回响应码
    payload = http.getString();//获取源码
    http.end();//结束请求
    DynamicJsonBuffer jsonBuffer;//实例化json解析对象
    JsonObject& json = jsonBuffer.parseObject(payload);
    String d1 = json["sysTime2"];//解析json
    if (d1 != "") {
      //判断不为空
      date = d1;
      hour = date.substring(10, 13).toInt();
      m = date.substring(14, 16).toInt();
      s = date.substring(17, 19).toInt();
      Serial.println(1);
    } else {
      gt2();//为空则调用另一个接口
      Serial.println(2);
    }
  }
}

void gt2()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    //{"success":"1","result":{"timestamp":"1628665198","datetime_1":"2021-08-11 14:59:58","datetime_2":"2021年08月11日 14时59分58秒","week_1":"3","week_2":"星期三","week_3":"周三","week_4":"Wednesday"}}
    http.begin(apiurl);
    http.addHeader("Content-Type", "text/plain");
    httpCode = http.GET();
    payload = http.getString();
    http.end();
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(payload);
    //与上一个接口大同小异
    String nmsl = json["result"]["datetime_1"];
    String nmcs = json["result"]["week_4"];
    if (nmsl != "" && nmcs != "") //预防接口奔溃
    {
      date = nmsl;
      day = nmcs;
      int year = date.substring(0,5).toInt();
      Serial.println(year);
      int mon = date.substring(6,8).toInt();
      Serial.println(mon);
      int d = date.substring(9,11).toInt();
      Serial.println(d);
      hour = date.substring(10, 13).toInt();
      m = date.substring(14, 16).toInt();
      s = date.substring(17, 19).toInt();
      clock_set_time(day,year,mon,d,hour,m,s);

    } else {
     clock_get_time();
    }
  }
}

void clock_get_time() {
  /*获取芯片时间*/
  Time t = rtc.time();
  day = dayAsString(t.day);
  char buf[50];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           t.yr, t.mon, t.date,
           t.hr, t.min, t.sec);
  date=buf;
}

/*
   天气获取
*/
int wd_i = 0;
void wd()
{
  /*
     获取天气
  */
  String line;
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  if (https.begin(*client, "https://devapi.heweather.net/v7/weather/now?gzip="))
  {
    int httpCode = https.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        line = https.getString();
      }
    }

    https.end();
  }
  /*
     对天气json进行解析
  */
  DynamicJsonBuffer jsonBuffer(1024);
  JsonObject& res_json = jsonBuffer.parseObject(line);
  int r1 = res_json["now"]["icon"]; //天气
  String r2 = res_json["now"]["temp"]; //温度
  String r3 = res_json["now"]["windDir"];//风向
  jsonBuffer.clear();

  /*
     判断天气情况是否为空
  */
  if (r1 != NULL && r2 != NULL && r3 != NULL) {
    tq  = r1;                                   //天气
    wendu = r2;                                 //温度
    fen_x = r3;                                 //风向
    wd_i = 0;//重置递归次数
  } else {
    /*
       限定递归次数
    */
    if (wd_i < 5) {
      wd_i++;
      delay(1000);
      wd();
    } else Serial.println("达到最大递归限制 无法获取天气");
  }

}


void show_tq(int col) {
  /*
     天气情况处理
     然后显示出来
  */
  if (col == 100 || col == 150) { //晴
    display.drawBitmap(32, 8, qing, 8, 8, 1);
  } else if (col == 102 || col == 101) { //云
    display.drawBitmap(32, 8, duo, 8, 8, 1);
    display.drawBitmap(41, 8, yun, 8, 8, 1);
  } else  if (col == 103 || col == 153) {
    //晴间多云
    display.drawBitmap(32, 8, qing, 8, 8, 1);
    display.drawBitmap(32 + 8, 8, jian, 8, 8, 1);
    display.drawBitmap(32 + 16, 8, duo, 8, 8, 1);
    display.drawBitmap(32 + 24, 8, yun, 8, 8, 1);
  } else if (col == 104 || col == 154) { //阴
    display.drawBitmap(32, 8, yin, 8, 8, 1);
  } else if (col >= 300 && col <= 301) { //阵雨
    display.drawBitmap(32, 8, zhen, 8, 8, 1);
    display.drawBitmap(32 + 8, 8, yu, 8, 8, 1);
  } else if (col >= 302 && col <= 303) { //雷阵雨
    display.drawBitmap(32, 8, lei, 8, 8, 1);
    display.drawBitmap(32 + 8, 8, zhen, 8, 8, 1);
    display.drawBitmap(32 + 16, 8, yu, 8, 8, 1);
  } else if (col == 304) {
    //冰雹
    display.drawBitmap(32, 8, bing, 8, 8, 1);
    display.drawBitmap(32 + 8, 8, bao, 8, 8, 1);
  } else if (col == 399 || col == 314 || col == 305 || col == 306 || col == 307 || col == 315 || col == 350 || col == 351) { //雨
    display.drawBitmap(32, 8, yu, 8, 8, 1);
  } else if ((col >= 308 && col <= 313) || (col >= 316 && col <= 318)) {
    //暴雨
    display.drawBitmap(32, 8, bao2, 8, 8, 1);
    display.drawBitmap(32 + 8, 8, yu, 8, 8, 1);
  } else if ((col >= 402 && col <= 406) || col == 409 || col == 410 || col == 400 || col == 401 || col == 408 || col == 499 || col == 456) {
    //雪
    display.drawBitmap(32, 8, xue, 8, 8, 1);
  } else if (col == 407 || col == 457) {
    //阵雪
    display.drawBitmap(32, 8, zhen, 8, 8, 1);
    display.drawBitmap(32 + 8, 8, xue, 8, 8, 1);
  } else if (col >= 500 && col <= 502) {
    //雾霾
    display.drawBitmap(32, 8, wu, 8, 8, 1);
    display.drawBitmap(32 + 8, 8, mai, 8, 8, 1);
  } else if (col >= 503 && col <= 508) {
    //沙尘暴
    display.drawBitmap(32, 8, sha, 8, 8, 1);
    display.drawBitmap(32 + 8, 8, chen, 8, 8, 1);
    display.drawBitmap(32 + 16, 8, bao, 8, 8, 1);
  } else if (col >= 509 && col <= 515) {
    //不适宜生存
    display.setCursor(32, 8);
    display.print("can't live");
  } else {
    //未知
    display.setCursor(32, 8);
    display.print("Unknow");
  }
  /*
     显示风向
  */
  if (fen_x == "东风") {
    display.drawBitmap(58, 0, dong, 10, 10, 1);
    display.drawBitmap(58 + 10, 0, feng, 10, 10, 1);
  } else if (fen_x == "西风") {
    display.drawBitmap(58, 0, xi, 10, 10, 1);
    display.drawBitmap(58 + 10, 0, feng, 10, 10, 1);
  } else if (fen_x == "北风") {
    display.drawBitmap(58, 0, bei, 10, 10, 1);
    display.drawBitmap(58 + 10, 0, feng, 10, 10, 1);
  } else if (fen_x == "南风") {
    display.drawBitmap(58, 0, nan_1, 10, 10, 1);
    display.drawBitmap(58 + 10, 0, feng, 10, 10, 1);
  } else  if (fen_x == "东北风") {
    display.drawBitmap(58, 0, dong, 10, 10, 1);
    display.drawBitmap(58 + 10, 0, bei, 10, 10, 1);
    display.drawBitmap(58 + 20, 0, feng, 10, 10, 1);
  } else if (fen_x == "西北风") {
    display.drawBitmap(58, 0, xi, 10, 10, 1);
    display.drawBitmap(58 + 10, 0, bei, 10, 10, 1);
    display.drawBitmap(58 + 20, 0, feng, 10, 10, 1);
  } else  if (fen_x == "东南风") {
    display.drawBitmap(58, 0, dong, 10, 10, 1);
    display.drawBitmap(58 + 10, 0, nan_1, 10, 10, 1);
    display.drawBitmap(58 + 20, 0, feng, 10, 10, 1);
  } else if (fen_x == "西南风") {
    display.drawBitmap(58, 0, xi, 10, 10, 1);
    display.drawBitmap(58 + 10, 0, nan_1, 10, 10, 1);
    display.drawBitmap(58 + 20, 0, feng, 10, 10, 1);
  }
}

String dayAsString(const Time::Day day) {
  /*处理对应的星期*/
  switch (day) {
    case Time::kSunday: return "Sunday";
    case Time::kMonday: return "Monday";
    case Time::kTuesday: return "Tuesday";
    case Time::kWednesday: return "Wednesday";
    case Time::kThursday: return "Thursday";
    case Time::kFriday: return "Friday";
    case Time::kSaturday: return "Saturday";
  }
  return "(unknown day)";
}

bool clock_set_time(String set_day, int set_year, int set_mon, int set_date, int set_hour, int set_m, int set_s) {
  /*设置ds1302时间*/
  if (set_day == "Monday") {
    Time t(set_year, set_mon, set_date, set_hour, set_m, set_s, Time::kMonday);
    rtc.time(t);
  } else if (set_day == "Tuesday") {
    Time t(set_year, set_mon, set_date, set_hour, set_m, set_s, Time::kTuesday);
    rtc.time(t);
  } else if (set_day == "Wednesday") {
    Time t(set_year, set_mon, set_date, set_hour, set_m, set_s, Time::kWednesday);
    rtc.time(t);
  } else if (set_day == "Thursday") {
    Time t(set_year, set_mon, set_date, set_hour, set_m, set_s, Time::kThursday);
    rtc.time(t);
  } else if (set_day == "Friday") {
    Time t(set_year, set_mon, set_date, set_hour, set_m, set_s, Time::kFriday);
    rtc.time(t);
  } else if (set_day == "Saturday") {
    Time t(set_year, set_mon, set_date, set_hour, set_m, set_s, Time::kSaturday);
    rtc.time(t);
  } else if (set_day == "Sunday") {
    Time t(set_year, set_mon, set_date, set_hour, set_m, set_s, Time::kSunday);
    rtc.time(t);
  } else {
    return 0;
  }
  return 1;
}

void set_led(){
	led_open = !led_open;

}
