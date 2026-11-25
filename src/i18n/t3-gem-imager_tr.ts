<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="tr_TR">
<context>
    <name>DfuDriver</name>
    <message>
        <location filename="../dfudriver.cpp" line="117"/>
        <source>Windows cannot reach the board because no suitable USB driver is bound to its DFU interface.&lt;br&gt;&lt;br&gt;The installer normally puts this driver in place. If it was skipped, re-run the T3 Gemstone Imager installer and leave &lt;b&gt;DFU driver (USB flashing)&lt;/b&gt; selected, then retry - the board does not need to be power cycled.&lt;br&gt;&lt;br&gt;The driver wanted is WinUSB, for &lt;b&gt;USB\VID_%1&amp;amp;PID_%2&lt;/b&gt;; it can also be installed by hand with Zadig.</source>
        <translation>Kartın DFU arayüzüne uygun bir USB sürücüsü bağlı olmadığı için Windows karta ulaşamıyor.&lt;br&gt;&lt;br&gt;Bu sürücüyü normalde kurulum programı yükler. Atlandıysa T3 Gemstone Imager kurulumunu yeniden çalıştırın, &lt;b&gt;DFU sürücüsü (USB ile yazma)&lt;/b&gt; seçili kalsın, sonra yeniden deneyin. Kartın gücünü kesip açmanız gerekmez.&lt;br&gt;&lt;br&gt;Gereken sürücü &lt;b&gt;USB\VID_%1&amp;amp;PID_%2&lt;/b&gt; için WinUSB; Zadig ile elle de kurulabilir.</translation>
    </message>
    <message>
        <location filename="../dfudriver.cpp" line="129"/>
        <source>&lt;br&gt;&lt;br&gt;The interface is currently claimed by the &lt;b&gt;%1&lt;/b&gt; driver.</source>
        <translation>&lt;br&gt;&lt;br&gt;Arayüz şu anda &lt;b&gt;%1&lt;/b&gt; sürücüsünde.</translation>
    </message>
</context>
<context>
    <name>DfuThread</name>
    <message>
        <location filename="../dfuthread.cpp" line="476"/>
        <location filename="../dfuthread.cpp" line="493"/>
        <source>Reading image from cache/local file (no download needed)...</source>
        <translation>İmaj önbellekten/yerel dosyadan okunuyor (indirme gerekmiyor)...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="816"/>
        <source>Fetching bootloader files...</source>
        <translation>Önyükleyici dosyaları alınıyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="1227"/>
        <source>Fetching bootloader list...</source>
        <translation>Önyükleyici listesi alınıyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="1213"/>
        <location filename="../dfuthread.cpp" line="1326"/>
        <source>Failed to download bootloader file: %1</source>
        <translation>Önyükleyici dosyası indirilemedi: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="434"/>
        <location filename="../dfuthread.cpp" line="481"/>
        <location filename="../dfuthread.cpp" line="500"/>
        <location filename="../dfuthread.cpp" line="514"/>
        <location filename="../dfuthread.cpp" line="775"/>
        <location filename="../dfuthread.cpp" line="818"/>
        <location filename="../dfuthread.cpp" line="833"/>
        <location filename="../dfuthread.cpp" line="837"/>
        <source>Cancelled</source>
        <translation>İptal edildi</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="569"/>
        <source>Download corrupt. Hash does not match.&lt;br&gt;The data already sent to the board is incomplete: write the image again before booting it.</source>
        <translation>İndirilen dosya bozuk. Hash eşleşmiyor.&lt;br&gt;Karta gönderilen veri eksik: kartı başlatmadan önce imajı yeniden yazın.</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="626"/>
        <source>unknown error</source>
        <translation>bilinmeyen hata</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="699"/>
        <source>Switching the board to USB mass storage mode...</source>
        <translation>Kart USB yığın depolama moduna geçiriliyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="748"/>
        <source>The board switched to USB mass storage mode, but no disk appeared within %1 seconds.&lt;br&gt;&lt;br&gt;Power off the board, set the boot switches to DFU mode again, restore power and retry.</source>
        <translation>Kart USB yığın depolama moduna geçti, ancak %1 saniye içinde disk görünmedi.&lt;br&gt;&lt;br&gt;Kartın gücünü kesin, önyükleme anahtarlarını yeniden DFU moduna alın, gücü verip yeniden deneyin.</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="769"/>
        <source>Device is writing to eMMC (do not power off)...</source>
        <translation>Kart eMMC&apos;ye yazıyor (gücü kesmeyin)...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="778"/>
        <source>The image was written, but the board did not leave USB mass storage mode after it was ejected, so the boot binaries may not have been updated.&lt;br&gt;Power off the board, set the boot switches to DFU mode again and retry.</source>
        <translation>İmaj yazıldı, ancak kart çıkarıldıktan sonra USB yığın depolama modundan çıkmadı; önyükleme dosyaları güncellenmemiş olabilir.&lt;br&gt;Kartın gücünü kesin, önyükleme anahtarlarını yeniden DFU moduna alın ve yeniden deneyin.</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="783"/>
        <source>The device is still writing to eMMC %1 minutes after the transfer finished.&lt;br&gt;Do NOT power off the board yet: check the serial console and wait until write activity stops.</source>
        <translation>Aktarım bittikten %1 dakika sonra kart hâlâ eMMC&apos;ye yazıyor.&lt;br&gt;Kartın gücünü henüz KESMEYİN: seri konsolu kontrol edin ve yazma etkinliği bitene kadar bekleyin.</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="880"/>
        <source>DFU transfer failed (%1), retrying...</source>
        <translation>DFU aktarımı başarısız oldu (%1), yeniden deneniyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="890"/>
        <source>DFU transfer failed while sending %1: %2&lt;br&gt;&lt;br&gt;%3</source>
        <translation>%1 gönderilirken DFU aktarımı başarısız oldu: %2&lt;br&gt;&lt;br&gt;%3</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="893"/>
        <source>DFU transfer failed while sending %1: %2&lt;br&gt;&lt;br&gt;Power off the board, set the boot switches to DFU mode again, restore power and retry.</source>
        <translation>%1 gönderilirken DFU aktarımı başarısız oldu: %2&lt;br&gt;&lt;br&gt;Kartın gücünü kesin, boot anahtarlarını yeniden DFU moduna alın, gücü verip tekrar deneyin.</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="921"/>
        <source>Sending image: %1 / %2 MB (%3%)</source>
        <translation>İmaj gönderiliyor: %1 / %2 MB (%3%)</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="1257"/>
        <source>Failed to fetch bootloader list and no fallback hashes available. Check your network connection and try again.</source>
        <translation>Önyükleyici listesi alınamadı ve yedek sağlama değerleri yok. Ağ bağlantınızı kontrol edip tekrar deneyin.</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="352"/>
        <location filename="../dfuthread.cpp" line="688"/>
        <source>Failed to create temporary file for DFU image</source>
        <translation>DFU imajı için geçici dosya oluşturulamadı</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="167"/>
        <source>Cannot customize this image: no partition table found at its start.</source>
        <translation>Bu imaj özelleştirilemiyor: başında bölüm tablosu bulunamadı.</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="175"/>
        <source>Cannot customize this image: its first partition ends at %1 MB, beyond the %2 MB that can be held back for customization.</source>
        <translation>Bu imaj özelleştirilemiyor: ilk bölümü %1 MB&apos;ta bitiyor, özelleştirme için ayrılabilecek %2 MB&apos;ın ötesinde.</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="188"/>
        <source>Failed to create temporary file for customizing the image: %1</source>
        <translation>İmajı özelleştirmek için geçici dosya oluşturulamadı: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="197"/>
        <source>Failed to open temporary file for customizing the image: %1</source>
        <translation>İmajı özelleştirmek için geçici dosya açılamadı: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="205"/>
        <source>Error writing temporary file for customizing the image: %1</source>
        <translation>İmajı özelleştirmek için geçici dosyaya yazılırken hata oluştu: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="231"/>
        <source>Failed to read back the customized image: %1</source>
        <translation>Özelleştirilmiş imaj geri okunamadı: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="239"/>
        <location filename="../dfuthread.cpp" line="625"/>
        <source>DFU transfer failed: %1</source>
        <translation>DFU aktarımı başarısız oldu: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="240"/>
        <location filename="../dfuthread.cpp" line="323"/>
        <location filename="../dfuthread.cpp" line="485"/>
        <source>transfer was not open</source>
        <translation>aktarım açık değildi</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="260"/>
        <source>Failed to initialise USB: %1</source>
        <translation>USB başlatılamadı: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="273"/>
        <source>No DFU device found (VID:0x%1 PID:0x%2 alt:%3) after %4 retries</source>
        <translation>%4 denemeden sonra DFU aygıtı bulunamadı (VID:0x%1 PID:0x%2 alt:%3)</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="283"/>
        <source>Failed to start the DFU transfer: %1</source>
        <translation>DFU aktarımı başlatılamadı: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="362"/>
        <source>Failed to open temporary file for DFU image</source>
        <translation>DFU imajı için geçici dosya açılamadı</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="408"/>
        <source>Initializing DFU...</source>
        <translation>DFU başlatılıyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="411"/>
        <source>DFU mode requires an image URL</source>
        <translation>DFU modu bir imaj URL&apos;si gerektiriyor</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="430"/>
        <source>Writing image to the board over USB mass storage...</source>
        <translation>İmaj USB yığın depolama üzerinden karta yazılıyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="459"/>
        <location filename="../dfuthread.cpp" line="524"/>
        <source>Image written to eMMC successfully! Power off the board and switch the boot mode to eMMC.</source>
        <translation>İmaj eMMC&apos;ye başarıyla yazıldı! Kartın gücünü kesin ve önyükleme modunu eMMC&apos;ye alın.</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="488"/>
        <source>DFU transfer failed at the end of the image: %1</source>
        <translation>DFU aktarımı imajın sonunda başarısız oldu: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="495"/>
        <source>Downloading image...</source>
        <translation>İmaj indiriliyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="504"/>
        <source>Customizing image...</source>
        <translation>İmaj özelleştiriliyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="508"/>
        <source>Failed to reopen image for customization: %1</source>
        <translation>Özelleştirme için imaj yeniden açılamadı: %1</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="831"/>
        <source>Sending bootloader files...</source>
        <translation>Önyükleyici dosyaları gönderiliyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="835"/>
        <source>Waiting for device to enter DFU mode...</source>
        <translation>Cihazın DFU moduna girmesi bekleniyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="474"/>
        <location filename="../dfuthread.cpp" line="519"/>
        <source>Sending image to device (this may take several minutes)...</source>
        <translation>İmaj cihaza gönderiliyor (bu birkaç dakika sürebilir)...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="937"/>
        <source>Sending %1...</source>
        <translation>%1 gönderiliyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="940"/>
        <source>%1 sent</source>
        <translation>%1 gönderildi</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="943"/>
        <source>Waiting for device to reconnect...</source>
        <translation>Cihazın yeniden bağlanması bekleniyor...</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="1204"/>
        <location filename="../dfuthread.cpp" line="1278"/>
        <source>Failed to create temp file for %1</source>
        <translation>%1 için geçici dosya oluşturulamadı</translation>
    </message>
    <message>
        <location filename="../dfuthread.cpp" line="1341"/>
        <source>Image not found: %1</source>
        <translation>İmaj bulunamadı: %1</translation>
    </message>
</context>
<context>
    <name>DownloadExtractThread</name>
    <message>
        <location filename="../downloadextractthread.cpp" line="210"/>
        <location filename="../downloadextractthread.cpp" line="404"/>
        <source>Error extracting archive: %1</source>
        <translation>Arşiv çıkarılırken hata oluştu: %1</translation>
    </message>
    <message>
        <location filename="../downloadextractthread.cpp" line="275"/>
        <source>Error mounting FAT32 partition</source>
        <translation>FAT32 bölümü bağlanırken hata oluştu</translation>
    </message>
    <message>
        <location filename="../downloadextractthread.cpp" line="295"/>
        <source>Operating system did not mount FAT32 partition</source>
        <translation>İşletim sistemi FAT32 bölümünü bağlamadı</translation>
    </message>
    <message>
        <location filename="../downloadextractthread.cpp" line="318"/>
        <source>Error changing to directory &apos;%1&apos;</source>
        <translation>Dizin değiştirirken hata oluştu &apos;%1&apos;</translation>
    </message>
</context>
<context>
    <name>DownloadThread</name>
    <message>
        <location filename="../downloadthread.cpp" line="119"/>
        <source>unmounting drive</source>
        <translation>Sürücü bağlantısı kesiliyor</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="139"/>
        <source>opening drive</source>
        <translation>Sürücü açılıyor</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="169"/>
        <source>Error running diskpart: %1</source>
        <translation>Diskpart çalıştırılırken hata oluştu: %1</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="190"/>
        <source>Error removing existing partitions</source>
        <translation>Mevcut bölümler kaldırılırken hata oluştu </translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="216"/>
        <source>Authentication cancelled</source>
        <translation>Kimlik doğrulama iptal edildi</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="219"/>
        <source>Error running authopen to gain access to disk device &apos;%1&apos;</source>
        <translation>&apos;%1&apos; disk aygıtına erişmek için authopen çalıştırılırken hata oluştu</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="222"/>
        <source>Reason: %1</source>
        <translation>Neden: %1</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="227"/>
        <source>Give &apos;T3 Gemstone Imager&apos; &lt;b&gt;Full Disk Access&lt;/b&gt; in Privacy &amp; Security, then quit and reopen the application - macOS only reads the permission at startup.</source>
        <translation>Gizlilik ve Güvenlik&apos;te &apos;T3 Gemstone Imager&apos;a &lt;b&gt;Tam Disk Erişimi&lt;/b&gt; verin, sonra uygulamadan çıkıp yeniden açın. macOS bu izni yalnızca açılışta okur.</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="249"/>
        <source>Cannot open storage device &apos;%1&apos;.</source>
        <translation>Depolama cihazı açılamıyor &apos;%1&apos;.</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="291"/>
        <source>discarding existing data on drive</source>
        <translation>sürücüdeki mevcut verileri sil</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="313"/>
        <source>zeroing out first and last MB of drive</source>
        <translation>sürücünün ilk ve son MB&apos;sini sıfırlama</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="319"/>
        <source>Write error while zero&apos;ing out MBR</source>
        <translation>MBR sıfırlanırken yazma hatası</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="331"/>
        <source>Write error while trying to zero out last part of card.&lt;br&gt;Card could be advertising wrong capacity (possible counterfeit).</source>
        <translation>Kartın son kısmını sıfırlamaya çalışırken yazma hatası. Kart yanlış kapasitenin tanımını yapıyor olabilir (olası sahte bölüm boyutu tanımı)</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="421"/>
        <source>starting download</source>
        <translation>indirmeye başlanıyor</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="481"/>
        <source>Error downloading: %1</source>
        <translation>İndirilirken hata oluştu: %1</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="679"/>
        <source>Access denied error while writing file to disk.</source>
        <translation>Dosyayı diske yazarken erişim reddedildi hatası</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="684"/>
        <source>Controlled Folder Access seems to be enabled. Please add both t3-gem-imager.exe and fat32format.exe to the list of allowed apps and try again.</source>
        <translation>Kontrollü Klasör Erişimi etkin görünüyor. Lütfen izin verilen uygulamalar listesine hem t3-gem-imager.exe&apos;yi hem de fat32format.exe&apos;yi ekleyin ve tekrar deneyin.</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="691"/>
        <source>Error writing file to disk</source>
        <translation>Dosyayı diske yazma hatası</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="713"/>
        <source>Download corrupt. Hash does not match</source>
        <translation>İndirme bozuk. Hash eşleşmiyor</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="725"/>
        <location filename="../downloadthread.cpp" line="777"/>
        <source>Error writing to storage (while flushing)</source>
        <translation>Depolama alanına yazma hatası (flushing sırasında)</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="732"/>
        <location filename="../downloadthread.cpp" line="784"/>
        <source>Error writing to storage (while fsync)</source>
        <translation>Depoya yazma hatası (fsync sırasında)</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="767"/>
        <source>Error writing first block (partition table)</source>
        <translation>İlk bloğu yazma hatası (bölüm tablosu)</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="846"/>
        <source>Error reading from storage.&lt;br&gt;SD card may be broken.</source>
        <translation>Depolamadan okuma hatası.&lt;br&gt;SD kart arızalı olabilir.</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="865"/>
        <source>Verifying write failed. Contents of SD card is different from what was written to it.</source>
        <translation>Yazma doğrulanamadı. SD kartın içeriği, üzerine yazılandan farklı.</translation>
    </message>
    <message>
        <location filename="../downloadthread.cpp" line="920"/>
        <source>Customizing image</source>
        <translation>İmaj özelleştiriliyor</translation>
    </message>
</context>
<context>
    <name>DriveFormatThread</name>
    <message>
        <location filename="../driveformatthread.cpp" line="63"/>
        <location filename="../driveformatthread.cpp" line="124"/>
        <location filename="../driveformatthread.cpp" line="185"/>
        <source>Error partitioning: %1</source>
        <translation>Bölümleme hatası: %1</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="84"/>
        <source>Error starting fat32format</source>
        <translation>fat32format başlatılırken hata oluştu</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="94"/>
        <source>Error running fat32format: %1</source>
        <translation>fat32format çalıştırılırken hata oluştu: %1</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="104"/>
        <source>Error determining new drive letter</source>
        <translation>Yeni sürücü harfi belirlenirken hata oluştu</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="109"/>
        <source>Invalid device: %1</source>
        <translation>Geçersiz cihaz: %1</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="146"/>
        <source>Error formatting (through udisks2)</source>
        <translation>Hatalı biçimlendirme (udisks2 aracılığıyla)</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="174"/>
        <source>Error starting sfdisk</source>
        <translation>sfdisk başlatılırken hata oluştu</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="199"/>
        <source>Partitioning did not create expected FAT partition %1</source>
        <translation>Bölümleme, beklenen FAT bölümünü oluşturmadı: %1</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="208"/>
        <source>Error starting mkfs.fat</source>
        <translation>mkfs.fat başlatılırken hata oluştu</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="218"/>
        <source>Error running mkfs.fat: %1</source>
        <translation>mkfs.fat çalıştırılırken hata oluştu: %1</translation>
    </message>
    <message>
        <location filename="../driveformatthread.cpp" line="225"/>
        <source>Formatting not implemented for this platform</source>
        <translation>Bu platform için biçimlendirme uygulanmadı</translation>
    </message>
</context>
<context>
    <name>ImageWriter</name>
    <message>
        <location filename="../imagewriter.cpp" line="349"/>
        <location filename="../imagewriter.cpp" line="1055"/>
        <source>Image found in cache, skipping download</source>
        <translation>İmaj önbellekte bulundu, indirme atlanıyor</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="335"/>
        <source>Storage capacity is not large enough.&lt;br&gt;Needs to be at least %1 GB.</source>
        <translation>Depolama kapasitesi yeterince büyük değil.&lt;br&gt;En az %1 GB olması gerekiyor</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="341"/>
        <source>Input file is not a valid disk image.&lt;br&gt;File size %1 bytes is not a multiple of 512 bytes.</source>
        <translation>Giriş dosyası geçerli bir disk imajı değil.&lt;br&gt;%1 bayt dosya boyutu 512 baytın katı değil.</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="808"/>
        <source>Downloading and writing image</source>
        <translation>İmaj indirme ve yazma</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="919"/>
        <source>DFU mode not selected</source>
        <translation>DFU modu seçilmedi</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="947"/>
        <source>Authentication failed or was cancelled.&lt;br&gt;DFU operation requires elevated privileges to access the USB device.</source>
        <translation>Kimlik doğrulama başarısız veya iptal edildi.&lt;br&gt;DFU işlemi USB cihazına erişmek için yükseltilmiş ayrıcalıklar gerektiriyor.</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="1099"/>
        <source>could not be created</source>
        <translation>oluşturulamadı</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="1118"/>
        <source>is not writable</source>
        <translation>yazılabilir değil</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="1128"/>
        <source>is RAM-backed</source>
        <translation>RAM üzerinde</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="1172"/>
        <source>Could not find a usable temporary directory to extract the image.&lt;br&gt;Tried: %1&lt;br&gt;This is a permissions problem, not a disk space problem.</source>
        <translation>İmajı açmak için kullanılabilir bir geçici klasör bulunamadı.&lt;br&gt;Denenenler: %1&lt;br&gt;Bu bir izin sorunu, disk alanı sorunu değil.</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="1180"/>
        <source>Not enough disk space to customize the image.&lt;br&gt;About %1 MB of free space is needed, but only %2 MB is available.&lt;br&gt;Free up disk space, or write the image without customization.</source>
        <translation>İmajı özelleştirmek için yeterli disk alanı yok.&lt;br&gt;Yaklaşık %1 MB boş alan gerekiyor, ancak yalnızca %2 MB var.&lt;br&gt;Disk alanı açın ya da imajı özelleştirmeden yazın.</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="1262"/>
        <source>Select image</source>
        <translation>Imaj seç</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="1437"/>
        <source>Error synchronizing time. Trying again in 3 seconds</source>
        <translation>Zaman senkronizasyonu sırasında hata oluştu. 3 saniye içinde tekrar denenecek</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="1449"/>
        <source>STP is enabled on your Ethernet switch. Getting IP will take long time.</source>
        <translation>STP Ethernet anahtarınızda etkin. IP almak uzun zaman alacak.</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="1701"/>
        <source>Would you like to prefill the wifi password from the system keychain?</source>
        <translation>Sistem anahtar zincirinden wifi parolasını doldurmak ister misiniz?</translation>
    </message>
</context>
<context>
    <name>LocalFileExtractThread</name>
    <message>
        <location filename="../localfileextractthread.cpp" line="34"/>
        <source>opening image file</source>
        <translation>imaj dosyası açılıyor</translation>
    </message>
    <message>
        <location filename="../localfileextractthread.cpp" line="39"/>
        <source>Error opening image file</source>
        <translation>Imaj dosyası açılırken hata oluştu</translation>
    </message>
</context>
<context>
    <name>MsgPopup</name>
    <message>
        <location filename="../MsgPopup.qml" line="121"/>
        <source>NO</source>
        <translation>HAYIR</translation>
    </message>
    <message>
        <location filename="../MsgPopup.qml" line="130"/>
        <source>YES</source>
        <translation>EVET</translation>
    </message>
    <message>
        <location filename="../MsgPopup.qml" line="139"/>
        <source>CONTINUE</source>
        <translation>DEVAM ET</translation>
    </message>
    <message>
        <location filename="../MsgPopup.qml" line="147"/>
        <source>QUIT</source>
        <translation>ÇIKIŞ</translation>
    </message>
</context>
<context>
    <name>OptionsPopup</name>
    <message>
        <location filename="../OptionsPopup.qml" line="27"/>
        <source>OS Customization</source>
        <translation>İşletim Sistemi Özelleştirme</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="67"/>
        <source>General</source>
        <translation>Genel</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="76"/>
        <source>Services</source>
        <translation>Hizmetler</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="82"/>
        <source>Options</source>
        <translation>Seçenekler</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="116"/>
        <source>Set hostname:</source>
        <translation>Hostname:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="145"/>
        <source>Set username and password</source>
        <translation>Kullanıcı adı ve parola</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="159"/>
        <source>Username:</source>
        <translation>Kullanıcı Adı:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="185"/>
        <source>Password is required!</source>
        <translation>Parola gerekli!</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="193"/>
        <location filename="../OptionsPopup.qml" line="270"/>
        <source>Password:</source>
        <translation>Parola:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="234"/>
        <source>Configure wireless LAN</source>
        <translation>Kablosuz LAN Ayarla</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="248"/>
        <source>SSID:</source>
        <translation>SSID:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="299"/>
        <source>Show password</source>
        <translation>Parolayı göster</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="310"/>
        <source>Hidden SSID</source>
        <translation>Gizli SSID</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="321"/>
        <source>Wireless LAN country:</source>
        <translation>Kablosuz LAN ülkesi:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="339"/>
        <source>Set locale settings</source>
        <translation>Yerel ayarlar</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="343"/>
        <source>Time zone:</source>
        <translation>Zaman dilimi:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="362"/>
        <source>Keyboard layout:</source>
        <translation>KLavye düzeni:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="381"/>
        <source>Set Disk Encryption (Cryptsetup)</source>
        <translation>Disk Şifrelemesini Ayarla (Cryptsetup)</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="391"/>
        <source>Disk Password:</source>
        <translation>Disk Parolası:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="432"/>
        <source>Write sdcard&apos;s image to eMMC on first boot</source>
        <translation>SD kartın imajını ilk açılışta eMMC&apos;ye yaz</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="457"/>
        <source>Enable SSH</source>
        <translation>SSH Etkinleştir</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="473"/>
        <source>Use password authentication</source>
        <translation>Parola doğrulamasını kullan</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="484"/>
        <source>Allow public-key authentication only</source>
        <translation>Sadece açık anahtar doğrulamasına izin ver</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="495"/>
        <source>Set authorized_keys for &apos;%1&apos;:</source>
        <translation>&apos;%1&apos; için authorized_keys ayarla:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="539"/>
        <source>Delete Key</source>
        <translation>Anahtarı Sil</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="558"/>
        <source>RUN SSH-KEYGEN</source>
        <translation>SSH-KEYGEN ÇALIŞTIR</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="568"/>
        <source>Add SSH Key</source>
        <translation>SSH Anahtarı Ekle</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="584"/>
        <source>Enable VNC</source>
        <translation>VNC Etkinleştir</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="590"/>
        <source>VNC Password is required!</source>
        <translation>VNC Parolası gerekli!</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="599"/>
        <source>VNC Password:</source>
        <translation>VNC Parolası:</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="618"/>
        <source>Gadgets</source>
        <translation>Donanım Modülleri</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="624"/>
        <source>Enable Storage Gadget</source>
        <translation>Depolama Modülünü Etkinleştir</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="630"/>
        <source>Enable Ethernet Gadget</source>
        <translation>Ethernet Modülünü Etkinleştir</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="636"/>
        <source>Enable Serial Gadgets</source>
        <translation>Seri Port Modüllerini Etkinleştir</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="661"/>
        <source>Play sound when finished</source>
        <translation>Bittiğinde ses çal</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="665"/>
        <source>Eject media when finished</source>
        <translation>Bittiğinde medyayı çıkar</translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="684"/>
        <source>Serial Port </source>
        <translation>Seri Port </translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="716"/>
        <source>Ethernet </source>
        <translation>Ethernet </translation>
    </message>
    <message>
        <location filename="../OptionsPopup.qml" line="760"/>
        <source>SAVE</source>
        <translation>KAYDET</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../linux/linuxdrivelist.cpp" line="126"/>
        <source>Internal SD card reader</source>
        <translation>Dahili SD kart okuyucu</translation>
    </message>
</context>
<context>
    <name>S:</name>
    <message>
        <location filename="../dependencies/xz-5.6.2/src/xz/args.c" line="641"/>
        <source></source>
        <comment>This is a translatable</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../dependencies/xz-5.6.2/src/xz/file_io.c" line="264"/>
        <source></source>
        <comment>When compression or decompression finishes,</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../dependencies/xz-5.6.2/src/xz/list.c" line="164"/>
        <source></source>
        <comment>Indicates that there is no integrity check.</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../dependencies/xz-5.6.2/src/xz/list.c" line="170"/>
        <source></source>
        <comment>Indicates that integrity check name is not known,</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../dependencies/xz-5.6.2/src/xz/list.c" line="736"/>
        <source></source>
        <comment>These are column headings. From Strms (Streams)</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../dependencies/xz-5.6.2/src/xz/list.c" line="1186"/>
        <source></source>
        <comment>%s is an integer. Only the plural form of this</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../dependencies/xz-5.6.2/src/xz/message.c" line="702"/>
        <source></source>
        <comment>This is the program name in the beginning</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../dependencies/xz-5.6.2/src/xz/message.c" line="1062"/>
        <source></source>
        <comment>The word &quot;literal&quot; in &quot;literal context bits&quot;</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../dependencies/xz-5.6.2/src/xz/message.c" line="1133"/>
        <source></source>
        <comment>This message indicates the bug reporting address</comment>
        <translation></translation>
    </message>
</context>
<context>
    <name>Transfer</name>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="93"/>
        <source>Transfer cancelled</source>
        <translation>Aktarım iptal edildi</translation>
    </message>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="118"/>
        <source>Cannot find serial port!</source>
        <translation>Seri port bulunamadı!</translation>
    </message>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="127"/>
        <source>Invalid serial port!</source>
        <translation>Geçersiz seri port!</translation>
    </message>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="144"/>
        <source>Cannot configure serial port!</source>
        <translation>Seri port yapılandırılamadı!</translation>
    </message>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="151"/>
        <source>Unable to open port: </source>
        <translation>Port açılamadı: </translation>
    </message>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="163"/>
        <source>Unable to open file: </source>
        <translation>Dosya açılamadı: </translation>
    </message>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="168"/>
        <source>Unable to open QFile: </source>
        <translation>QFile açılamadı: </translation>
    </message>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="201"/>
        <source>Incorrect start of XMODEM transmission (0x</source>
        <translation>XMODEM aktarımının başlangıcı hatalı (0x</translation>
    </message>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="208"/>
        <source>Timeout</source>
        <translation>Zaman aşımı</translation>
    </message>
    <message>
        <location filename="../dependencies/qtxmodem/transfer.cpp" line="318"/>
        <source>Status timeout</source>
        <translation>Durum zaman aşımı</translation>
    </message>
</context>
<context>
    <name>UseSavedSettingsPopup</name>
    <message>
        <location filename="../UseSavedSettingsPopup.qml" line="46"/>
        <source>Use OS customization?</source>
        <translation>İşletim sistemi özelleştirme kullanılsın mı?</translation>
    </message>
    <message>
        <location filename="../UseSavedSettingsPopup.qml" line="98"/>
        <source>Would you like to apply OS customization settings?</source>
        <translation>İşletim sistemi özelleştirme ayarlarını uygulamak ister misiniz?</translation>
    </message>
    <message>
        <location filename="../UseSavedSettingsPopup.qml" line="139"/>
        <source>NO</source>
        <translation>HAYIR</translation>
    </message>
    <message>
        <location filename="../UseSavedSettingsPopup.qml" line="120"/>
        <source>NO, CLEAR SETTINGS</source>
        <translation>HAYIR, AYARLARI TEMİZLE</translation>
    </message>
    <message>
        <location filename="../UseSavedSettingsPopup.qml" line="130"/>
        <source>YES</source>
        <translation>EVET</translation>
    </message>
    <message>
        <location filename="../UseSavedSettingsPopup.qml" line="107"/>
        <source>EDIT SETTINGS</source>
        <translation>AYARLARI DÜZENLE</translation>
    </message>
</context>
<context>
    <name>WriteInPlaceThread</name>
    <message>
        <location filename="../writeinplacethread.cpp" line="26"/>
        <source>Error comm start with simpbootp server</source>
        <translation>simpbootp sunucusuyla iletişim başlatılamadı</translation>
    </message>
    <message>
        <location filename="../writeinplacethread.cpp" line="70"/>
        <source>Uniflash not supported for this board. This error indicates corrupted json configuration!</source>
        <translation>Bu kart için Uniflash desteklenmiyor. Bu hata, json yapılandırmasının bozuk olduğunu gösterir!</translation>
    </message>
    <message>
        <location filename="../writeinplacethread.cpp" line="188"/>
        <source>Download extract failed!</source>
        <translation>İndirilen dosya açılamadı!</translation>
    </message>
    <message>
        <location filename="../writeinplacethread.cpp" line="196"/>
        <source>Error connecting Simpbootp server</source>
        <translation>Simpbootp sunucusuna bağlanılamadı</translation>
    </message>
    <message>
        <location filename="../writeinplacethread.cpp" line="214"/>
        <location filename="../writeinplacethread.cpp" line="232"/>
        <source>Error starting communication with board</source>
        <translation>Kartla iletişim başlatılamadı</translation>
    </message>
    <message>
        <location filename="../writeinplacethread.cpp" line="221"/>
        <location filename="../writeinplacethread.cpp" line="241"/>
        <source>Error sending file with XMODEM: </source>
        <translation>Dosya XMODEM ile gönderilemedi: </translation>
    </message>
    <message>
        <location filename="../writeinplacethread.cpp" line="282"/>
        <source>TFTP server failed to send image file!</source>
        <translation>TFTP sunucusu imaj dosyasını gönderemedi!</translation>
    </message>
    <message>
        <location filename="../writeinplacethread.cpp" line="288"/>
        <source>Process cancelled by user. Please power cycle the board before retrying!</source>
        <translation>İşlem kullanıcı tarafından iptal edildi. Yeniden denemeden önce kartın gücünü kesip yeniden verin!</translation>
    </message>
</context>
<context>
    <name>main</name>
    <message>
        <location filename="../main.qml" line="1550"/>
        <source>Writing... %1%</source>
        <translation>Yazılıyor... %1%</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1548"/>
        <source>Preparing image... %1%</source>
        <translation>İmaj hazırlanıyor... %1%</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1212"/>
        <location filename="../main.qml" line="2202"/>
        <source>DFU Mode (eMMC)</source>
        <translation>DFU Modu (eMMC)</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1114"/>
        <location filename="../main.qml" line="2191"/>
        <source>Network Boot</source>
        <translation>Network Boot</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1124"/>
        <source>Writes the image to the card&apos;s internal eMMC memory over the network</source>
        <translation>İmajı ağ üzerinden kartın dahili eMMC belleğine yazar</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1454"/>
        <source>No storage device selected.&lt;br&gt;Please choose a storage device and try again.</source>
        <translation>Depolama aygıtı seçilmedi.&lt;br&gt;Lütfen bir depolama aygıtı seçip tekrar deneyin.</translation>
    </message>
    <message>
        <location filename="../main.qml" line="134"/>
        <source>CHOOSE DEVICE</source>
        <translation>AYGIT SEÇİN</translation>
    </message>
    <message>
        <location filename="../main.qml" line="160"/>
        <location filename="../main.qml" line="554"/>
        <source>Operating System</source>
        <translation>İşletim sistemi</translation>
    </message>
    <message>
        <location filename="../main.qml" line="171"/>
        <location filename="../main.qml" line="2038"/>
        <source>CHOOSE OS</source>
        <translation>İŞLETİM SİSTEMİ SEÇİN</translation>
    </message>
    <message>
        <location filename="../main.qml" line="183"/>
        <source>Select this button to change the operating system</source>
        <translation>İşletim sistemini değiştirmek için bu butonu seçin</translation>
    </message>
    <message>
        <location filename="../main.qml" line="197"/>
        <location filename="../main.qml" line="956"/>
        <source>Storage</source>
        <translation>Depolama</translation>
    </message>
    <message>
        <location filename="../main.qml" line="330"/>
        <source>Network not ready yet</source>
        <translation>Ağ henüz hazır değil</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1021"/>
        <source>No storage devices found</source>
        <translation>Depolama cihazı bulunamadı</translation>
    </message>
    <message>
        <location filename="../main.qml" line="208"/>
        <location filename="../main.qml" line="1692"/>
        <source>CHOOSE STORAGE</source>
        <translation>HEDEF SEÇİN</translation>
    </message>
    <message>
        <location filename="../main.qml" line="24"/>
        <source>T3 Gemstone Image Writer %1</source>
        <translation>T3 Gemstone Image Writer %1</translation>
    </message>
    <message>
        <location filename="../main.qml" line="122"/>
        <location filename="../main.qml" line="453"/>
        <source>Board</source>
        <translation>Kart</translation>
    </message>
    <message>
        <location filename="../main.qml" line="146"/>
        <source>Select this button to choose your target board</source>
        <translation>Hedef kartı seçmek için bu butonu seçin</translation>
    </message>
    <message>
        <location filename="../main.qml" line="222"/>
        <source>Select this button to change the destination storage device</source>
        <translation>Hedef depolama cihazını değiştirmek için bu butonu seçin</translation>
    </message>
    <message>
        <location filename="../main.qml" line="267"/>
        <source>CANCEL WRITE</source>
        <translation>YAZMAYI İPTAL ET</translation>
    </message>
    <message>
        <location filename="../main.qml" line="270"/>
        <location filename="../main.qml" line="1539"/>
        <location filename="../main.qml" line="1558"/>
        <source>Cancelling...</source>
        <translation>İptal ediliyor...</translation>
    </message>
    <message>
        <location filename="../main.qml" line="282"/>
        <source>CANCEL VERIFY</source>
        <translation>DOĞRULAMAYI İPTAL ET</translation>
    </message>
    <message>
        <location filename="../main.qml" line="285"/>
        <location filename="../main.qml" line="1581"/>
        <location filename="../main.qml" line="1725"/>
        <source>Finalizing...</source>
        <translation>Bitiriliyor...</translation>
    </message>
    <message>
        <location filename="../main.qml" line="294"/>
        <source>Next</source>
        <translation>İleri</translation>
    </message>
    <message>
        <location filename="../main.qml" line="300"/>
        <source>Select this button to start writing the image</source>
        <translation>İmajı yazmaya başlamak için bu butonu seçin</translation>
    </message>
    <message>
        <location filename="../main.qml" line="320"/>
        <source>Using custom repository: %1</source>
        <translation>Özelleştirilmiş depo kullanılıyor: %1</translation>
    </message>
    <message>
        <location filename="../main.qml" line="339"/>
        <source>Keyboard navigation: &lt;tab&gt; navigate to next button &lt;space&gt; press button/select item &lt;arrow up/down&gt; go up/down in lists</source>
        <translation>Klavye navigasyonu: &lt;tab&gt; bir sonraki butona git &lt;space&gt; butona bas / öğe seç &lt;arrow up/down&gt; listelerde yukarı/aşağı git</translation>
    </message>
    <message>
        <location filename="../main.qml" line="360"/>
        <source>Language: </source>
        <translation>Dil: </translation>
    </message>
    <message>
        <location filename="../main.qml" line="383"/>
        <source>Keyboard: </source>
        <translation>Klavye: </translation>
    </message>
    <message>
        <location filename="../main.qml" line="497"/>
        <source>[ All ]</source>
        <translation>[ Tümü ]</translation>
    </message>
    <message>
        <location filename="../main.qml" line="652"/>
        <source>Back</source>
        <translation>Geri</translation>
    </message>
    <message>
        <location filename="../main.qml" line="653"/>
        <source>Go back to main menu</source>
        <translation>Ana menüye dön</translation>
    </message>
    <message>
        <location filename="../main.qml" line="899"/>
        <source>Released: %1</source>
        <translation>Yayın: %1</translation>
    </message>
    <message>
        <location filename="../main.qml" line="909"/>
        <source>Cached on your computer</source>
        <translation>Bilgisayarınızda önbelleğe alındı
</translation>
    </message>
    <message>
        <location filename="../main.qml" line="911"/>
        <source>Local file</source>
        <translation>Yerel dosya</translation>
    </message>
    <message>
        <location filename="../main.qml" line="912"/>
        <source>Online - %1 GB download</source>
        <translation>Çevrimiçi -%1 GB indir</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1056"/>
        <source>Exclude System Drives</source>
        <translation>Sistem Sürücülerini Hariç Tut</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1268"/>
        <source>gigabytes</source>
        <translation>gigabayt</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1270"/>
        <location filename="../main.qml" line="1332"/>
        <source>Mounted as %1</source>
        <translation>%1 olarak bağlandı.</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1318"/>
        <source>GB</source>
        <translation>GB</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1334"/>
        <source>[WRITE PROTECTED]</source>
        <translation>[YAZMA KORUMALI]</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1336"/>
        <source>SYSTEM</source>
        <translation>SİSTEM</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1414"/>
        <source>Are you sure you want to quit?</source>
        <translation>Çıkmak istediğine emin misin?</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1426"/>
        <source>Warning</source>
        <translation>Uyarı</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1460"/>
        <source>Preparing to write...</source>
        <translation>Yazmaya hazırlanıyor...</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1471"/>
        <source>All existing data on &apos;%1&apos; will be erased.&lt;br&gt;Are you sure you want to continue?</source>
        <translation>&apos;%1&apos; üzerindeki mevcut tüm veriler silinecek.&lt;br&gt;Devam etmek istediğinizden emin misiniz?</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1487"/>
        <source>Update available</source>
        <translation>Güncelleme bulunuyor</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1488"/>
        <source>There is a newer version of Imager available.&lt;br&gt;Would you like to visit the website to download it?</source>
        <translation>İmager&apos;ın daha yeni bir sürümü var. &lt;br&gt; İndirmek için web sitesini ziyaret etmek ister misiniz?</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1545"/>
        <source>Downloading... %1%</source>
        <translation>İndiriliyor... %1%</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1584"/>
        <source>Verifying... %1%</source>
        <translation>Doğrulanıyor... %1%</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1595"/>
        <source>Preparing to write... (%1)</source>
        <translation>Yazmaya hazırlanıyor... (%1)</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1640"/>
        <source>Error</source>
        <translation>Hata</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1648"/>
        <source>Write Successful</source>
        <translation>Yazma Başarılı</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1649"/>
        <location filename="../imagewriter.cpp" line="750"/>
        <source>Erase</source>
        <translation>Sil</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1650"/>
        <source>&lt;b&gt;%1&lt;/b&gt; has been erased&lt;br&gt;&lt;br&gt;You can now remove the SD card from the reader</source>
        <translation>&lt;b&gt;%1&lt;/b&gt; silindi &lt;br&gt;&lt;br&gt; Artık SD kartı okuyucudan çıkarabilirsiniz</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1679"/>
        <source>&lt;b&gt;%1&lt;/b&gt; has been written to &lt;b&gt;%2&lt;/b&gt;&lt;br&gt;&lt;br&gt;You can now remove the SD card from the reader</source>
        <translation>&lt;b&gt;%1&lt;/b&gt; &lt;b&gt;%2&lt;/b&gt;&lt;br&gt;&lt;br&gt; üzerine yazıldı. Artık SD kartı okuyucudan çıkarabilirsiniz</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1859"/>
        <source>Error parsing os_list.json</source>
        <translation>os_list.json ayrıştırma hatası</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="751"/>
        <source>Format card as FAT32</source>
        <translation>Kartı FAT32 olarak biçimlendir</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="757"/>
        <source>Use custom</source>
        <translation>Özel imaj kullan</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="758"/>
        <source>Select a custom .img from your computer</source>
        <translation>Bilgisayarınızdan özel bir .img seçin</translation>
    </message>
    <message>
        <location filename="../main.qml" line="2112"/>
        <source>Connect an USB stick containing images first.&lt;br&gt;The images must be located in the root folder of the USB stick.</source>
        <translation>Önce imajlar içeren bir USB bellek bağlayın.&lt;br&gt; İmajlar USB belleğin kök klasöründe bulunmalıdır.</translation>
    </message>
    <message>
        <location filename="../main.qml" line="2228"/>
        <source>SD card is write protected.&lt;br&gt;Push the lock switch on the left side of the card upwards, and try again.</source>
        <translation>SD kart yazma korumalı. &lt;br&gt; Kartın sol tarafındaki kilit anahtarını yukarı itin ve tekrar deneyin.</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1222"/>
        <source>Program the device via USB DFU (Device Firmware Update)</source>
        <translation>Cihazı USB DFU (Aygıt Yazılımı Güncelleme) üzerinden programla</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1391"/>
        <location filename="../main.qml" line="1398"/>
        <location filename="../main.qml" line="1405"/>
        <source>Boot Mode Switch</source>
        <translation>Önyükleme Modu Anahtarı</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1392"/>
        <source>Configure the boot mode switches for DFU Boot as shown in the image.</source>
        <translation>Önyükleme modu anahtarlarını görselde gösterildiği gibi DFU Önyükleme için yapılandırın.</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1399"/>
        <source>DFU programming completed successfully!&lt;br&gt;&lt;br&gt;After powering off the card, set the boot mode switches to eMMC Boot as shown in the image. Upon restoring power, the system will boot automatically.</source>
        <translation>DFU programlama başarıyla tamamlandı!&lt;br&gt;&lt;br&gt;Kartı kapattıktan sonra, önyükleme modu anahtarlarını görselde gösterildiği gibi eMMC Önyüklemesine ayarlayın. Güç yeniden verildiğinde sistem otomatik olarak başlayacaktır.</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1674"/>
        <source>&lt;b&gt;%1&lt;/b&gt; has been written to &lt;b&gt;%2&lt;/b&gt;&lt;br&gt;&lt;br&gt;You can now remove the SD card from the reader. With the board powered off, insert the SD card and set the boot mode switches to SD Boot as shown in the image. Upon restoring power, the system will boot from the SD card.</source>
        <translation>&lt;b&gt;%1&lt;/b&gt;, &lt;b&gt;%2&lt;/b&gt; üzerine yazıldı&lt;br&gt;&lt;br&gt;SD kartı okuyucudan çıkarabilirsiniz. Kart kapalıyken SD kartı takın ve önyükleme modu anahtarlarını görselde gösterildiği gibi SD Önyüklemesine ayarlayın. Güç yeniden verildiğinde sistem SD karttan başlayacaktır.</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1415"/>
        <source>T3 Gemstone Imager is still busy.&lt;br&gt;Are you sure you want to quit?</source>
        <translation>T3 Gemstone Imager hâlâ meşgul.&lt;br&gt;Çıkmak istediğinizden emin misiniz?</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1446"/>
        <location filename="../main.qml" line="2215"/>
        <source>Starting DFU operation...</source>
        <translation>DFU işlemi başlatılıyor...</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1469"/>
        <source>Image will be sent to device via DFU.&lt;br&gt;Are you sure you want to continue?</source>
        <translation>İmaj, DFU aracılığıyla cihaza gönderilecek.&lt;br&gt;Devam etmek istediğinizden emin misiniz?</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1561"/>
        <source>Sending... %1%</source>
        <translation>Gönderiliyor... %1%</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1616"/>
        <source>Waiting for authentication to access USB device...</source>
        <translation>USB cihaza erişim için kimlik doğrulama bekleniyor...</translation>
    </message>
    <message>
        <location filename="../main.qml" line="1668"/>
        <source>&lt;b&gt;%1&lt;/b&gt; has been written to &lt;b&gt;%2&lt;/b&gt;&lt;br&gt;&lt;br&gt;The process is complete. You can connect to the board via the serial port.</source>
        <translation>&lt;b&gt;%1&lt;/b&gt;, &lt;b&gt;%2&lt;/b&gt; üzerine yazıldı.&lt;br&gt;&lt;br&gt;İşlem tamamlandı. Karta seri port üzerinden bağlanabilirsiniz.</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="770"/>
        <location filename="../simpdhcp.cpp" line="536"/>
        <source>Interface name (Ex: Ethernet)</source>
        <translation>Arayüz adı (Örn: Ethernet)</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="771"/>
        <location filename="../simpdhcp.cpp" line="537"/>
        <source>interface</source>
        <translation>interface</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="773"/>
        <location filename="../simpdhcp.cpp" line="539"/>
        <source>Interface speed (1/10/100/1000).</source>
        <translation>Arayüz hızı (1/10/100/1000).</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="774"/>
        <location filename="../simpdhcp.cpp" line="540"/>
        <source>speed</source>
        <translation>speed</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="776"/>
        <location filename="../simpdhcp.cpp" line="542"/>
        <source>Duplex type (half/full).</source>
        <translation>Çift yönlü iletişim türü (half/full).</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="777"/>
        <location filename="../simpdhcp.cpp" line="543"/>
        <source>duplex</source>
        <translation>duplex</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="779"/>
        <location filename="../simpdhcp.cpp" line="545"/>
        <source>Server ip (this pc) address (Ex: 10.42.0.1).</source>
        <translation>Sunucu (bu bilgisayar) IP adresi (Örn: 10.42.0.1).</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="780"/>
        <location filename="../simpdhcp.cpp" line="546"/>
        <source>server-ip</source>
        <translation>server-ip</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="782"/>
        <location filename="../simpdhcp.cpp" line="548"/>
        <source>Client ip (served pc) address (Ex: 10.42.0.2).</source>
        <translation>İstemci (hizmet verilen cihaz) IP adresi (Örn: 10.42.0.2).</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="783"/>
        <location filename="../simpdhcp.cpp" line="549"/>
        <source>offered-ip</source>
        <translation>offered-ip</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="785"/>
        <location filename="../simpdhcp.cpp" line="551"/>
        <source>Server name</source>
        <translation>Sunucu adı</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="786"/>
        <location filename="../simpdhcp.cpp" line="552"/>
        <source>server-name</source>
        <translation>server-name</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="788"/>
        <location filename="../simpdhcp.cpp" line="554"/>
        <source>Bootfile name which will be pulled from tftp server (Ex: tiboot3.bin).</source>
        <translation>TFTP sunucusundan alınacak önyükleme dosyasının adı (Örn: tiboot3.bin).</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="789"/>
        <location filename="../simpdhcp.cpp" line="555"/>
        <source>bootfile</source>
        <translation>bootfile</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="791"/>
        <location filename="../simptftp.cpp" line="14"/>
        <source>TFTP block size</source>
        <translation>TFTP blok boyutu</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="792"/>
        <location filename="../simptftp.cpp" line="15"/>
        <source>blocksize</source>
        <translation>blocksize</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="794"/>
        <location filename="../simptftp.cpp" line="18"/>
        <source>TFTP port.</source>
        <translation>TFTP portu.</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="795"/>
        <location filename="../simptftp.cpp" line="19"/>
        <source>port</source>
        <translation>port</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="797"/>
        <location filename="../simptftp.cpp" line="22"/>
        <source>Copy all source files into &lt;directory&gt;.</source>
        <translation>Tüm kaynak dosyaları &lt;directory&gt; klasörüne kopyala.</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="798"/>
        <location filename="../simptftp.cpp" line="23"/>
        <source>directory</source>
        <translation>directory</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="800"/>
        <location filename="../simptftp.cpp" line="25"/>
        <source>Serve only &lt;filename&gt; and exit.</source>
        <translation>Yalnızca &lt;filename&gt; dosyasını sun ve çık.</translation>
    </message>
    <message>
        <location filename="../simpbootp.cpp" line="801"/>
        <location filename="../simptftp.cpp" line="26"/>
        <source>filename</source>
        <translation>filename</translation>
    </message>
</context>
</TS>
