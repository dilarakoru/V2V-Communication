# V2V Communication - EEBL (Electronic Emergency Brake Light) System
## Projenin Amacı
Bu proje, EEBL (Electronic Emergency Brake Light) sistemini simüle ederek, lider aracın ani fren yapma durumunda takip eden araca bu durumu bildiren bir iletişim ağı kurar. Proje, araçlar arası (V2V) iletişim teknolojisini kullanarak, ani fren olaylarını takipçi araca bildirir ve geri bildirim mekanizması ile lider araca bu bilgiyi iletir.

Proje, acil fren olayını tespit eden ve bu bilgiyi başka bir araca ileten bir C++ uygulaması olarak tasarlanmıştır. UDP protokolü üzerinden mesaj iletilir ve iki farklı bilgisayar (lider ve takipçi araçlar) arasında veri alışverişi sağlanır.

## Proje Özeti
Lider araç: Acil fren durumunu algılar ve takip eden araca acil fren mesajı gönderir.
Takipçi araç: Lider aracın acil fren mesajını alır ve lider araca "Acil fren mesajı alındı" geri bildirimi gönderir.
EEBL Olayı Nedir?
EEBL (Electronic Emergency Brake Light), öndeki bir aracın ani fren yapması durumunda, arkasındaki araçlara uyarı gönderen bir güvenlik sistemidir. Bu proje, bu uyarı sistemini V2V iletişimi üzerinden simüle eder. Öndeki aracın hızı, fren pozisyonu, yavaşlama hızı ve iki araç arasındaki mesafe baz alınarak, ani fren durumları takipçi araca iletilir.

## Acil Fren Olayı Algoritması
Projede, acil fren olayını tespit etmek için aşağıdaki parametreler kullanılır:

Araç Hızı (speed): 0-90 km/h aralığında. Eğer hız 70 km/h'nin üzerindeyse, acil fren durumu olabilir.

Yavaşlama Hızı (deceleration): 0-10 m/s² aralığında. Yavaşlama hızı 8 m/s²'den büyükse, acil fren durumu olabilir.

Fren Pedalı Pozisyonu (brake_position): 0-100% aralığında. Fren pedal pozisyonu %70'in üzerinde olmalıdır.

İki Araç Arasındaki Mesafe (distance): Mesafe 1000 metreden küçükse ve diğer koşullar sağlanıyorsa acil fren mesajı gönderilir.

Bu koşullar sağlandığında, lider araç takip eden araca "Emergency Brake Activated!" mesajını gönderir.

## Zorluklar ve Potansiyel Çözümler
1. UDP Üzerinde Veri İletimindeki Zorluklar
Zorluk: UDP, TCP'ye göre daha hızlı olmasına rağmen, güvenilir bir veri iletim protokolü değildir. Veri paketleri kaybolabilir veya yanlış sırayla gelebilir.
Çözüm: Proje, veri kaybı olmayan bir yerel ağda çalışmak üzere tasarlandı. Eğer daha güvenli bir iletim gerekiyorsa, TCP gibi daha güvenilir bir protokol kullanılabilir.
2. Ağ Bağlantısı
Zorluk: İki farklı cihazın aynı yerel ağda iletişim kurması zor olabilir.
Çözüm: Bilgisayarların aynı IP aralığında ve UDP ile iletişim kurabilmesi için ağ yapılandırmalarının doğru olduğundan emin olunmalıdır.
3. Zamanlama ve Gecikme
Zorluk: Araçlar arasında gecikme ve zamanlama hataları oluşabilir.
Çözüm: UDP hızlı bir iletişim sağlar, ancak daha sık doğrulama mekanizmaları eklenebilir.
## Kullanılan Teknolojiler
1. C++: Projenin temel dili olarak seçilmiştir.
2. Winsock2: UDP üzerinden iki bilgisayar arasında veri iletimi yapmak için kullanılır.
3. MinGW: Windows üzerinde C/C++ kodlarını derlemek ve çalıştırmak için kullanılan geliştirme ortamı.
4. ESP32 modülü
   
## Laptop2Laptop Projesini Çalıştırma Adımları
### 1. MinGW Kurulumu
MinGW (Minimalist GNU for Windows) kullanarak projeyi derlemek için şu adımları izleyin:

[MinGW İndir](https://sourceforge.net/projects/mingw/)

Kurulum sırasında g++, make, mingw32-base gibi paketlerin yüklendiğinden emin olun.

### 2. Winsock2 Kütüphanesinin Kullanımı
Bu projede UDP ile veri iletimi sağlamak için Winsock2 kütüphanesi kullanılmıştır. Projeyi derlerken, -lws2_32 linker'ını eklemeniz gerekmektedir:
```cpp
  g++ leader.cpp -o V2V_leader.exe -lws2_32
  
  g++ follower.cpp -o V2V_follower.exe -lws2_32
```
### 3. Projeyi Derlemek ve Çalıştırmak
Proje dosyalarını terminal veya komut satırı üzerinden derleyebilirsiniz:
```cpp
  g++ leader.cpp -o V2V_leader.exe -lws2_32
   
  g++ follower.cpp -o V2V_follower.exe -lws2_32
```
Lider bilgisayar: V2V_leader.exe dosyasını çalıştırın.

Takipçi bilgisayar: V2V_follower.exe dosyasını çalıştırın.

### 4. Araç Parametrelerini Girmek
Lider araçta hız, fren pedal pozisyonu ve yavaşlama bilgilerini girin.

Takipçi araç, lider aracın acil fren durumunu algılayarak geri bildirimde bulunacaktır.
   
## Esp2Esp Projesini Çalıştırma Adımları
### 1. Arduino IDE Kurulumu
### 2. Esp32 board manager
### 3.

## Mesaj Akışı
Lider Araç:
Lider araç, takipçiden konum bilgilerini alır, iki araç arasındaki mesafeyi hesaplar ve acil fren koşullarını değerlendirir.
Eğer acil fren durumu algılanırsa, takipçi araca acil fren mesajı gönderir.

Takipçi Araç:
Takipçi araç, lider araçtan gelen acil fren mesajını alır ve bu mesajı aldıktan sonra lider araca "Acil fren mesajı alındı" bilgisini geri bildirir.

## Dosya Yapısı
### Laptop2Laptop 
1. leader.cpp: Lider aracın acil fren mesajını gönderen kodu içerir.
2. follower.cpp: Takipçi aracın acil fren mesajını alıp geri bildirimde bulunmasını sağlar.
3. Makefile.win: Projeyi derlemek için kullanılan dosya.
4. .exe dosyaları: Derlenmiş çalıştırılabilir dosyalar.
### Esp2Esp
1. leader.cpp: Lider aracın acil fren mesajını gönderen kodu içerir.
2. follower.cpp: Takipçi aracın acil fren mesajını alıp geri bildirimde bulunmasını sağlar.
3. Makefile.win: Projeyi derlemek için kullanılan dosya.
4. .exe dosyaları: Derlenmiş çalıştırılabilir dosyalar.

## Sonuç
Bu proje, lider ve takipçi araçlar arasında EEBL (Electronic Emergency Brake Light) simülasyonunu başarıyla gerçekleştirir. Lider araç ani fren yaptığında, takipçi araç uyarılır ve lider araca geri bildirim sağlar. Bu proje, V2V iletişiminin nasıl çalıştığını ve UDP protokolü ile hızlı veri iletiminin nasıl sağlanacağını gösterir.
