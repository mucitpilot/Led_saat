//MUCİT PİLOT 2020 // LedSaat v1.0 // 
// Saat, Tarih, Gün, Sıcaklık ve Nem gösteren Led Saat Uygulaması
//LDR ile ortam ışığına göre ekran parklaklığı
//Buton ile gösterilen bilgiyi seçme
//RTC Modül olarak 1302 kullanılmıştır.
//Sıcaklık sensörü olarak DHT11 kullanılmıştır.

//gerekli kütüphaneler
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <virtuabotixRTC.h>   
#include <avr/pgmspace.h>
#include <DHT.h>

//kendi oluşturduğumuz font dosyasını ekliyoruz
#include "Fonts_data.h"

//LED modül tanımlamaları ve PIN tanımlamaları
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW  //kullanılan modül tipi. 4ü bir arada modeller için FC16_HW kullanın
#define MAX_DEVICES 3 //kaç modül bağlı olduğu
#define CLK_PIN   13 //pinler
#define DATA_PIN  11
#define CS_PIN    10
#define BUTON_PIN    9
#define DHT_PIN    2 //sicaklik sensörü hangi pine bağlı
#define DHTTYPE DHT11 //sensör tipi DHT11 veya DHT22

// bir sıcaklık-nem sensörü nesnesi oluşturduk
DHT sicakliksensor(DHT_PIN, DHTTYPE);

//saat nesnesini ve gerekli değişkenleri oluşturduk
virtuabotixRTC myRTC(6, 7, 8); //clk,dat,rst pinleri
char gunsay;
char gun[]="";
char tarih[11];
char saat[9];

// Bir adet Parola nesnesi yaratıyoruz
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);


// Kayan Yazı Paramatreleri
uint8_t scrollSpeed = 50;    // Kayma hızı, rakam küçüldükçe hız artar
//Giriş ve çıkışta kayma efektlerini seçin
textEffect_t scrollEffectin = PA_SCROLL_LEFT; //PA_SCROLL_LEFT,PA_SCROLL_RIGHT,PA_SCROLL_UP,PA_SCROLL_DOWN
textEffect_t scrollEffectout = PA_SCROLL_LEFT; //PA_SCROLL_LEFT,PA_SCROLL_RIGHT,PA_SCROLL_UP,PA_SCROLL_DOWN
//metin ne tarafa hizalanacak
textPosition_t scrollAlign = PA_CENTER;//PA_CENTER,PA_LEFT,PA_RIGHT
uint16_t scrollPause = 0; // Metin kaç milisaniye sabit olarak gösterimde kalacak

uint8_t  inFX, outFX; //giriş ve çıkışta uygulanacak efektler
textEffect_t  effect[] = //efektleri tuttuğumuz dizi
{

  //kütüphanede tanımlı efektler...
  PA_PRINT, //0
  PA_SCAN_HORIZ,//1
  PA_SCROLL_LEFT,//2
  PA_WIPE, //3
  PA_SCROLL_UP_LEFT,//4
  PA_SCROLL_UP,//5
  PA_OPENING_CURSOR,//6
  PA_GROW_UP,//7
  PA_MESH,//8
  PA_SCROLL_UP_RIGHT,//9
  PA_BLINDS,//10
  PA_CLOSING,//11
  PA_RANDOM,//12
  PA_GROW_DOWN,//13
  PA_SCAN_VERT,//14
  PA_SCROLL_DOWN_LEFT,//15
  PA_WIPE_CURSOR,//16
  PA_DISSOLVE,//17
  PA_OPENING,//18
  PA_CLOSING_CURSOR, //19
  PA_SCROLL_DOWN_RIGHT,//20
  PA_SCROLL_RIGHT,//21
  PA_SLICE,//22
  PA_SCROLL_DOWN,//23
};


#define	BUF_SIZE	75 //genel olarak kullanacağımız metin uzunluğu
char mesajyaz[BUF_SIZE]; //metinleri mesajyaz değişkeni ile yazdıracağız
const uint16_t WAIT_TIME = 0; //ilk açılış mesajı için yazının ekranda sabit bekleme süresi
String mesaj="Mucit Pilot LedSaat"; //ilk açılış mesajı
//ekran gösterim sıralaması ve menu butonu için gerekli değişkenleri tanımladık
int a=0;
int menu=0;

void setup()
{
  Serial.begin(57600);
  // Saat modülünü ilk aldığınızda bir seferlik doğru saati kaydetmek için:
  // saniye, dakika, saat, haftanin kacinci günü, ayın kaçıncı günü, ay, yıl olacak şekilde
 //myRTC.setDS1302Time(30, 22, 02, 3, 12, 8, 2020);  //komutunu çalıştırın sonra tekrar comment yapın veya silin
  P.begin();//parola kayan yazı nesnesini başlatıyoruz
  sicakliksensor.begin();//sicaklik sensörü nesnesini başlatıyoruz
  //mesajların gösterilmesi için gerekli ilk taımlamayı yaptık
  P.displayText(mesajyaz, scrollAlign, scrollSpeed, scrollPause, scrollEffectin, scrollEffectout); //inFX, outFX
  inFX=0; //açılış mesajının giriş çıkış efektlerini seçtik
  outFX=0;
  P.setIntensity(5);//açılış mesajının parlaklık seviyesini seçtik (0-15 arası)
  mesaj.toCharArray(mesajyaz,BUF_SIZE);//toCharArray komutu ile mesajı yazdırılabilecek tek karakterlerden oluşan char dizisine çevirdik //ÖNEMLİ
  P.setFont(kucukFont);//açılış mesajı için font tipi
  pinMode(BUTON_PIN, INPUT);//butonun pinmodunu tanımladık
}

void loop()
{ 
  while (digitalRead(BUTON_PIN)!=HIGH){  //butona basılmadığı sürece saat animasyonlarını oynat  
  if(P.displayAnimate()) //animasyonlarımız bu satır ve displayReset komutu arasında oynar
  {

   menu=0; //buton sayacını sıfırlıyourz

    //LDR sensörden bilgiyi okuyoryuz
   int isik = analogRead(A0);
   int parlaklik=map(isik,0,1000,0,9);//okuduğumuz değeri 0-9 parlaklık aralığına uyarladık
   P.setIntensity(parlaklik); //bu parlaklığı animasyona yazdırdık
   P.setSpeed(100); // animasyondaki oynama hızını tanımladık

   myRTC.updateTime(); //RTC saat nesnesinden biligiyi aldık  

   if (a==90){a=0;} //sayaç 90'a ulaşınca tekrar sıfırlıyoruz

   //bu kısımda sayacın 5' göre modunu alıp mod değerinin 0-1-2-3-4 olmasına göre 5 farklı metin içeriği gösterimini sıra ile oynatacağız
   if (a%5==0){
    
     saatyazdir();//ekrana saati yazdıran fonksiyonumuz
   }
   
   else if (a%5==1){
     P.setSpeed(75);
     tarihyazdir();//ekrana tarihi yazdıran fonksiyonumuz
   }
   else if (a%5==2){
    gunyazdir();//ekrana günü yazdıran fonksiyonumuz
   
   }

   else if (a%5==3){
    sicaklikyazdir();//ekrana sıcaklık yazdıran fonksiyonumuz
   
   }
   else { 
     nemyazdir();//ekrana nemi yazdıran fonksiyonumuz
    
   } 
    
   a++;//sayacı arttırdık
   
   P.displayReset(); //animasyonlar bu noktadan sonra tekrar yukarı başa dönecek

  }//if display animate'in sonu
  
  digitalRead(BUTON_PIN);//while içinde sonsuz dönmesin diye buton durumunu tekrar okuduk
 }//while'ın sonu

//bu kısımda butonun basılmasına göre yapılacak işlemleri hallediyoruz
 if (digitalRead(BUTON_PIN)==HIGH){ //eğer butona basılmış ise
  delay(100);//butona basmalarda sadece tek basma algılaması için koydum yoksa sayacı 2-3 arttırabilir.
  

  int sayi=menu%5;//menu sayacının değerinin de 5'e göre modunu aldık çünkü 5 bilgimiz var
  a=sayi+1;//butonu bırakınca yukarıda animasyona baştan başlaması için a sayacını sıfırladık
  P.displayClear();//ekranı temizledik çünkü butona bastığımız için yeni bilgi yazacağız
  switch (sayi){
     case 0 :
      menu++;//sayacı bir arttırdık
      saatyazdir();
    break;
     case 1 :
      menu++;//sayacı bir arttırdık
      tarihyazdir();
    break;
       case 2 :
       menu++;//sayacı bir arttırdık
      gunyazdir();
    break;
      case 3 :
      menu++;//sayacı bir arttırdık
      sicaklikyazdir();
    break;
      case 4 :
      menu++;//sayacı bir arttırdık
      nemyazdir();
    break;
    
    
    }
  P.displayReset(); //animasyonu başa döndürmek için
  }

}//void loop'un sonu


//Yardımcı Fonksiyonları tanımlıyoruz
//saati yazdırdığımız fonkisyon
void saatyazdir(){
  //saat hanesini al
  int myhour=myRTC.hours;
  String gecici1;
  //gelen veri tek haneli ise başına sıfır koymak için
  if (myhour < 10) { 
    gecici1='0'+String(myhour);
    }
  else {gecici1=String(myhour);}
 // Serial.println(hourss);
  
  //dakika hanesini al
  int myminute=myRTC.minutes;
  String gecici2;
  //gelen veri tek haneli ise başına sıfır koymak için
  if (myminute < 10) {
    gecici2='0'+String(myminute);
    }
  else {gecici2=String(myminute);}
  //Serial.println(minutess);
  
  //saniye hanesini al
  int mysecond=myRTC.seconds;
  String gecici3;
  //gelen veri tek haneli ise başına sıfır koymak için
  if (mysecond < 10) {
    gecici3='0'+String(mysecond);
    }
  else {gecici3=String(mysecond);}
  //Serial.println(secondss);

 String tamsaat=gecici1+':'+gecici2;
 tamsaat.toCharArray(mesajyaz,BUF_SIZE);//oluşturduğumuz string metni Char dizisine çevirmeliyiz
 inFX=5; //giriş efekti
 outFX=23;//çıkış efekti     
 P.setTextEffect(effect[inFX], effect[outFX]);//efektleri uygula
 P.setPause(5000); //metin ekrranda sabit kaç milisaniye kalacak
    
}

//tarihi yazdırdığımız fonkisyon
void tarihyazdir(){
  int myyear=myRTC.year;
  String gecici1=String(myyear);
  //gecici1=gecici1.substring(2,4);
  //Serial.println(years);

  int mymonth=myRTC.month;
  String gecici2;
  if (mymonth < 10) {
    gecici2='0'+String(mymonth);
    }
  else {gecici2=String(mymonth);}
  //Serial.println(months);

  int myday=myRTC.dayofmonth;
  String gecici3;
  if (myday < 10) {
    gecici3='0'+String(myday);
    }
  else {gecici3=String(myday);}
  //Serial.println(dayss);

 String tarih=gecici3+'.'+gecici2+'.'+gecici1;

 tarih.toCharArray(mesajyaz,BUF_SIZE);
 inFX=2;
 outFX=2;     
 P.setTextEffect(effect[inFX], effect[outFX]);
 P.setPause(0);

}
//günü yazdırdığımız fonkisyon
void gunyazdir(){
  

  int myday=myRTC.dayofweek;
  String gun_gecici;
  switch (myday) {
    case 0 :
      gun_gecici="PAZAR";
    break;
    case 1:
      gun_gecici="PAZARTESi";
    break;
    case 2:
      gun_gecici="SALI";
    break;
    case 3:
      gun_gecici="CARSAMBA";
    break;
    case 4:
      gun_gecici="PERSEMBE";
    break;
    case 5:
      gun_gecici="CUMA";
    break;   
    case 6:
      gun_gecici="CUMARTESi";
    break;            
  }

 gun_gecici.toCharArray(mesajyaz,BUF_SIZE);
 inFX=21;
 outFX=21;     
 P.setTextEffect(effect[inFX], effect[outFX]);
 P.setPause(0);

}


//sıcaklığı yazdırdığımız fonkisyon
void sicaklikyazdir(){
  
  float derece=sicakliksensor.readTemperature();
  String gecici1=String(derece);
  gecici1=gecici1.substring(0,2);//okunan değerin ilk iki hanesini almak için
  Serial.println(gecici1);

 String sicak=gecici1+' '+"°C";

 sicak.toCharArray(mesajyaz,BUF_SIZE);
 inFX=6;
 outFX=6;     
 P.setTextEffect(effect[inFX], effect[outFX]);
 P.setPause(3000);

}
//nemi yazdırdığımız fonkisyon
void nemyazdir(){
  float nem=sicakliksensor.readHumidity();
  
  String gecici1=String(nem);
  gecici1=gecici1.substring(0,2);//ilk iki haneyi almak için
 
 String yuzde=String(char(37));//% işaretini yazdırabilmek için kullandım
 String nems=yuzde+' '+gecici1;
 Serial.println(nems);
 nems.toCharArray(mesajyaz,BUF_SIZE);
 inFX=13;
 outFX=7;     
 P.setTextEffect(effect[inFX], effect[outFX]);
 P.setPause(3000);

}

//saat modülünüzün doğru bilgi okuyup okumaduğunu kontrol etmek için kullanabilirsiniz
//void zamanyazdir(){
//                                                               
//
//
////Serial.print(myRTC.dayofmonth);   
//  Serial.println(myRTC.dayofweek);   
//  Serial.print(myRTC.dayofmonth);                                                                   
//  Serial.print("/");                                                                                     
//  Serial.print(myRTC.month);                                                                             
//  Serial.print("/");                                                                                    
//  Serial.print(myRTC.year);                                                                            
//  Serial.print("  ");                                                                                    
//  Serial.print(myRTC.hours);                                                                            
//  Serial.print(":");                                                                                    
//  Serial.print(myRTC.minutes);                                                                          
//  Serial.print(":");                                                                                    
//  Serial.println(myRTC.seconds);                                                                        
////Serial.print(":");                                                                                    
////Serial.println(myRTC.dayofweek);                                                                           
//
//}
