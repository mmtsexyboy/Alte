# Alte Text Editor

Alte is a lightweight, fast, and modern text editor designed for performance and a clean user experience. It aims to provide essential features for developers and writers, with a focus on speed, customizability, and beautiful rendering of text.

(Alte : یک اپلیکیشن تکس ادیتور به شدت سبک و به شدت ساده و به شدت کاربردی)

For the detailed vision, philosophy, and future goals of the Alte project, please see [VISION.md](VISION.md).

## Features (Current & Planned)

*   Cross-platform (initially Linux)
*   High performance, capable of handling large files (using a Rope data structure)
*   Syntax highlighting for various programming languages
*   Theming support
*   Plugin architecture (planned)
*   UTF-8 and RTL language support (e.g., Persian, Arabic)
*   C++20 core with Qt6 (fallback to Qt5) for the UI

## Building from Source

### Prerequisites

*   A C++20 compliant compiler (e.g., GCC, Clang)
*   CMake (version 3.16 or higher)
*   Qt (version 6 or 5). Qt6 is preferred.
    *   Required Qt modules: Core, Gui, Widgets

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone <repository_url>
    cd alte
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure with CMake:**
    ```bash
    cmake ..
    ```
    (CMake will try to find Qt6 first. If not found, it will look for Qt5.)

4.  **Compile:**
    ```bash
    make
    ```
    (Or your chosen build system's command, e.g., `ninja`)

The executable `Alte` will be created in the `build` directory.

## Installation (Linux)

A DEB package can be created for easier installation on Debian-based Linux distributions:

1.  Follow the build steps above.
2.  From the `build` directory, run:
    ```bash
    cpack -G DEB
    ```
This will generate a `.deb` file (e.g., `alte-0.1.0-Linux.deb`) which can be installed using your system's package manager (e.g., `sudo dpkg -i alte-0.1.0-Linux.deb` followed by `sudo apt-get install -f` if there are dependency issues).

The editor will typically be installed to `/usr/local/bin` or `/opt/` and a `.desktop` file will be added for application menus.

## Contributing

Contributions are welcome! Please refer to the [VISION.md](VISION.md) for the overall direction. (Further contributing guidelines can be added here later).

مهم ترین نکته ؟؟ 
مهمم ترین نکته این است که این هانباید جلوی دست کاربر را بگیرد بلکه هر موقع خواست میتواند به سرعتی عظیم مثلا با زدن شورت کت و یا چیز دیگر میتواند این را فعال کند 
اولویت ما مینیمالیسم فوق گرافیک و سوپر زیبا است  یک چیز رویایی میخواهم

۱. حالت تایپ‌نویس (Typewriter Mode)
وقتی کاربر تایپ می‌کند، خط فعلی همیشه وسط صفحه نگه‌داشته می‌شود و بقیه متن بالا و پایین حرکت می‌کنند. این حالت برای نویسنده‌ها بسیار جذاب است و تمرکز را بالا می‌برد.
کاربر باید بتواند با مثلا زدن یک شورت کت این را فعال کند 

۲. نمایش آمار زنده (Live Stats)
نمایش زنده تعداد کلمات، کاراکترها، زمان تقریبی خواندن/نوشتن، و حتی نمودار ساده از روند پیشرفت تایپ در گوشه پایین یا به صورت Pop-up کوچک.

۳. حالت تاریک خودکار بر اساس ساعت سیستم
ویرایشگر به صورت خودکار شب‌ها تم تاریک را فعال و روزها تم روشن را فعال کند.

۴. هایلایت هوشمند خط فعلی
خط فعلی با افکت جذاب یا رنگ متفاوت مشخص شود تا چشم کاربر همیشه بداند کجاست.
اما باید به شدت نرم و زیبا باشد نباید معلوم باشد فقط باید ضمیر ناخود اگاه کاربر را تحریک کند  و نه انکه جلوی دست و پای کاربر باشد  و یا موجب جلب توجه شود

۶. پشتیبانی از Drag & Drop برای باز کردن فایل
کاربر بتواند هر فایل متنی را با کشیدن و رها کردن، به‌راحتی باز کند.

۸. حالت Focus برای سطر یا پاراگراف فعلی
جز خط یا پاراگرافی که کاربر روی آن کار می‌کند، بقیه متن کمی محو یا کم‌رنگ شود (برای تمرکز بیشتر).
این هم باید به شدت نرم و سبک باشدو اصلا هیچ کس نفهمد همچین چیزی هست فقط به مقدار کمی تمرکز را بهتر کند


۱. جستجوی سریع با کلید میانبر
پنجره ان  را باید خودما پیاده سازی کنیم چون نباید بالا و یا تیتل و یا تایتل و یا چیزی داشته باشد باید با تم و سبک ما هم خوان باشد باید در موقع نیاز خودش بسته شود 
نباید جلوی دست و پای کاربر باشد 


۱۰. تایمر تمرکز (Pomodoro Timer)
اضافه کردن یک تایمر Pomodoro کوچک داخل ویرایشگر برای افزایش بازدهی کاربر (مثلاً ۲۵ دقیقه کار، ۵ دقیقه استراحت).
این هم باید نباید جلوی دست و پای کاربر باشد و به شدت ساده و مینیمال باشد و ای اصلا پیاده سازی نکن! اگر نمیتوان یهب شدت مینیمال و کاربر پسند بسازی و جالب 


۲. Zen Mode (حالت آرامش مطلق)
با یک شورت‌کات، همه چیز (منوها، پنل‌ها، حتی نوار وضعیت) مخفی شود و فقط متن وسط صفحه باشد.
بازگشت به حالت عادی هم با همان شورت‌کات.
این هم باید با یک شورت کت باشد و به شدت کاربر پسند و بینظیر


اسکرول باز به شدت شخصی سازی شده و به شدت کاربر پسند


۷. Scratchpad
یک فضای موقت و پروازنده (Floating)، هر وقت کاربر شورت‌کات بزند یک باکس مینیمال باز می‌شود برای نوشتن یادداشت سریع، بدون دخالت در فایل اصلی.
این جایی ذخیره میشود و فقط یک فایل برای هر وقت 
