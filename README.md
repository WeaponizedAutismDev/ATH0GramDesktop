# ATH0Gram

![ATH0Gram Logo](.github/ATH0Gram.png) 

A fork of [AyuGram Desktop](https://github.com/AyuGram/AyuGramDesktop) for personal use and experimentation.

## Features

- Blackjack
- Hookers
- [Ayugram's](https://docs.ayugram.one/desktop/) original recipe with 11 herbs and spices. 

<details>

<summary>More Details</summary>

## Preferences screenshots

<img src='.github/demos/demo1.png' width='268'>
<img src='.github/demos/demo2.png' width='268'>
<img src='.github/demos/demo3.png' width='268'>
<img src='.github/demos/demo4.png' width='268'>

### Windows

#### Self-built

Make sure you have these components installed with VS Build Tools:

- C++ MFC latest (x86 & x64)
- C++ ATL latest (x86 & x64)
- latest Windows 11 SDK
- The exact python version specified in the build docs
- Ensure you have ATL libs installed that match your version of Visual studios MSVC - Select these in either the VS build tools or VS2022 isntaller, when you select c++ destkop dev tools.
- You will need to use X64 Native Tools Command Prompt for VS 2022
- The combinded X86 and X64 will break the build make sure you use the correct x64 command prompt, you can find it from the Start menu VS folder in Windows under "all apps"
- If you dont have the correct verison command prompt available read [Microsoft's guide](https://learn.microsoft.com/en-us/cpp/build/how-to-enable-a-64-bit-visual-cpp-toolset-on-the-command-line?view=msvc-170) on how to enable it.

Follow [official guide](https://github.com/WeaponizedAutismDev/ATH0GramDesktop/blob/dev/docs/building-win-x64.md) if you want to
build by yourself.

### Any other Linux distro

Follow the [official guide](https://github.com/WeaponizedAutismDev/ATH0GramDesktop/blob/dev/docs/building-linux.md).

## Credits

### Telegram clients

- [AyuGram Desktop](https://github.com/Ayugram/AyuGramDesktop)
- [MaterialGram](https://github.com/kukuruzka165/materialgram)
- [Telegram Desktop](https://github.com/telegramdesktop/tdesktop)
- [Kotatogram](https://github.com/kotatogram/kotatogram-desktop)
- [64Gram](https://github.com/TDesktop-x64/tdesktop)
- [Forkgram](https://github.com/forkgram/tdesktop)

### Libraries used

- [JSON for Modern C++](https://github.com/nlohmann/json)
- [SQLite](https://github.com/sqlite/sqlite)
- [sqlite_orm](https://github.com/fnc12/sqlite_orm)

### Icons

- [FluentUI System Icons](https://github.com/microsoft/fluentui-system-icons)
- [iconify.design](https://icon-sets.iconify.design/)

### Bots

- [TelegramDB](https://t.me/tgdatabase) for username lookup by ID (until closing free inline mode at 25 May 2025)
- [usinfobot](https://t.me/usinfobot) for username lookup by ID

</details>