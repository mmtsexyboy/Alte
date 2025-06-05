# Alte
a LINUX  text EDITOR
Alte : یک اپلیکیشن تکس ادیتور به شدت سبک و به شدت ساده و به شدت کاربردی 

مشکلات هر متن ادیتوری را برسی کن در لینوکس و چیزی بساز که ان ایراد ها را ندارد : مثلا gedit وقتی فایل بزرگ باز میکنی کراش میکنه ( احتمالا سیستم تشخیص حجم و lazy load نداره ) یا مثلا ویرایشگز های متن دیگر لینوکس بسیار ززشت هستند یا مثلا ویرایشگر اتم پیچیده گی های غیر ضروری دارد 

این اپلیکیشن باید تم ابل باشد + پلاگین ساختن برای ان خیلی ساده باشد + پلاگین ها سرعت ان را کم نکنند 

+ با ترفند هایی خاص و به شدت زیرکانه و بسیار-خلاثانه باید سریع و کم مصرف باشد 

زبان های مختلف را باید بتواند بر اساس سینتکس به نرمی بی نظیر و خیلی خیلی زیبا نشان دهد 

تم های مختلف

از راست نویس و چپ نویس مثلا فارسی و عربی را بینقص نشان دهد 

شورت کت های زیاد و ساده و کار امد


یک شاهکار بساز و خودت برای خودت پرامت بنویس 

رابط گرافیکی بی نظیر برای من به شدتی بی وصف مهم است از هر جا که سیستم و هوش تو تصمیم میگیرد شروع کن

از CPP استفاده کن میخوام سرعتش بی نظیر باشه +
پوشه بندی دایرکتوری ها و فایل ها رو جوری قرار بده که گیج نشی و همه چیز واضح باشد 

من باید بتوانم با مثلا eddy ان را با یک کلیک نصب بکنم 


در هر مرحله و در پایان کار کاملا برسی که که ایا باگی دارد و اگر داشت اصلاح ساز 

بسیار باید کاربر پسند باشد +


با کمترین کد ها بیشتری عظمت هارو خلق کن 


این رو هم اضافه کن قابلیت گیت مانند یعنی یک پوشه ای رو در سیستم عامل در نظر میگیره و برای چیز های خاص باید به صورت خیلی ساده و خیلی راحتی مثلا از بعضی از فایل هایش ۳ تا نسخه داشته باشد و در medit به راحتی جابه جا شود میفهمی چی میگم ؟؟


الگوریتمی پیاده کن که مطالب را فراموش نکنی  +


الگوریتم تشخیص زبان و بعد از نشخیص وسواسی و دقیق زبان پیشنهاد دهنده خودکار رو ایجاد کن و کلید وازه های هر زبان برنامه نویسی رو در پوشه ای در قایل های جدا قرار بده هر زبان دارای سینتکس و نحوه نمایش رنگی خود باید باشد این بخش باید خیلی حرفه ای تمییز و کاربر پسند باشد و سپس ان فایل ها را به دقتی عظیم پر کن و سپس باز دیباگ کن و سپس پر کن و سپس باز دیباگ کن 


زبان برنامه CPP



فایل بندی .DEB را پیاده سازی کن
این پروزه را شروع کن 

## Installation Instructions

You can install the `alte-0.1.0-Linux.deb` package using the command line or a graphical tool.

**Command-line installation:**

1. Open your terminal.
2. Navigate to the directory containing the `.deb` file.
3. Run the following command to install the package:
   ```bash
   sudo dpkg -i build_temp/alte-0.1.0-Linux.deb
   ```
4. If you encounter any dependency issues, run the following command to fix them:
   ```bash
   sudo apt-get install -f
   ```

**Graphical installation:**

You can also install the package by opening the `build_temp/alte-0.1.0-Linux.deb` file with a graphical package installer like GDebi or Eddy.

## Rebuilding the .deb Package

If you need to rebuild the `.deb` package from source, follow these steps:

1. Ensure you are in the project's root directory.
2. If a build directory (e.g., `build_temp`) doesn't exist or you want a clean build, create it and navigate into it. Otherwise, just navigate into the existing build directory:
   ```bash
   # For a clean build
   mkdir build_temp && cd build_temp
   # Or if build_temp already exists
   # cd build_temp
   ```
3. Configure the project using CMake. Make sure your `CMakeLists.txt` is in the parent directory (the project root):
   ```bash
   cmake ..
   ```
4. Build the package using CPack. This will generate the `.deb` file:
   ```bash
   cpack -G DEB
   ```
   Alternatively, you might be able to use:
   ```bash
   make package
   ```
5. The new `.deb` package will be located in the build directory (e.g., `build_temp`).
