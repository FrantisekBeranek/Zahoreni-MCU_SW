@echo off

:: open git manager
cd "D:\Users\FrantisekBeranek\AppData\Local\GitHubDesktop\"
start GitHubDesktop.exe

:: open stm32CUBE_IDE
D:
cd "D:\ST\STM32CubeIDE_1.7.0\STM32CubeIDE\"
start stm32cubeide.exe "D:\Users\FrantisekBeranek\Dokumenty\TSE\projekty\zahoreni\verze 3\SW\Zahoreni-MCU_SW\Zahoreni_zdroju\.cproject"

exit