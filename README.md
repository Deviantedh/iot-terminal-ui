# IoT Terminal UI

Интерактивный игровой IoT-терминал на базе `ESP8266 NodeMCU v3` с TFT-экраном,
резистивным touch, физическими кнопками, Wi-Fi, слот-игрой и серверной
интеграцией.

Проект начинался как UI-платформа для небольшого TFT-дисплея, а в текущем
состоянии представляет собой законченную embedded-систему: устройство умеет
подключаться к Wi-Fi, сохранять настройки, показывать интерфейс, запускать игру,
работать с балансом и получать результаты spin через backend API.

## Что реализовано

- Сенсорный UI для `ST7789 320x240` и `XPT2046`.
- Физические кнопки `MENU` и `POWER` через `PCF8574`.
- Экранная клавиатура для ввода Wi-Fi.
- Сканирование Wi-Fi сетей, подключение и сохранение credentials в `LittleFS`.
- Автоподключение после старта и reconnect после standby.
- `HOME`, `MENU`, `MODE SELECT`, `GAME`, `BALANCE`, `PROFILE`, `TOP UP`, `SETTINGS`, `TESTS`.
- Слот-машина в двух режимах: классический `3 reels` и расширенный `5 reels + paylines`.
- Баланс, ставки, выплаты, звуковые эффекты и отдельный экран баланса.
- Серверный API-клиент для аккаунтов, авторизации, баланса и результатов spin.
- Mock backend на Python/aiohttp и OpenAPI-контракт.
- Dirty regions / partial redraw для более отзывчивой отрисовки.
- Sleep / standby режимы без deep sleep.
- Project-local настройка `TFT_eSPI`, без ручного редактирования глобальных файлов библиотеки.

## Аппаратная часть

Основные компоненты:

- `ESP8266 NodeMCU v3`
- `ST7789 320x240`
- `XPT2046 touch`
- `PCF8574` по I2C
- пассивный зуммер
- кнопки `POWER` и `MENU`

Текущая распиновка:

| Узел | Подключение |
| --- | --- |
| Touch CS | `D2` |
| Touch IRQ | `D1` |
| PCF8574 SDA | `GPIO3 / RX` |
| PCF8574 SCL | `GPIO1 / TX` |
| PCF8574 address | `0x20` |
| Buzzer | `D0 / GPIO16` |

Так как `GPIO1/GPIO3` используются под I2C для `PCF8574`, Serial Monitor в
рабочем режиме не используется. Если открыть Serial Monitor с этой проводкой,
USB-UART будет видеть I2C-трафик как мусорные символы. Логи прошивки по
умолчанию отключены через `APP_SERIAL_LOGGING_ENABLED = 0`.

## Экраны устройства

- `HOME` - главный экран с NTP-временем, кнопками `PLAY` и `MENU`, статусом игрока/сервера.
- `MENU` - переходы в `PROFILE`, `TOP UP`, `SETTINGS`.
- `PROFILE` - выбор аккаунта, загружаемого с backend.
- `TOP UP` - заготовка под пополнение.
- `MODE SELECT` - выбор режима игры.
- `GAME` - слот-машина, ставка, spin, auto-play, баланс.
- `BALANCE` - отдельный экран баланса и побед.
- `SETTINGS` - Wi-Fi, touch diagnostics, sound, tests.
- `TESTS` - сервисный экран для проверки звука.

## Игра

Игровая часть вынесена в `SlotGame` и используется экраном `GAME`.

Поддерживаются два режима:

- `Classic` - 3 барабана, 1 ряд, простой выигрыш по совпадению трёх символов.
- `Advanced` - 5 барабанов, 3 ряда, 20 линий выплат.

Есть:

- выбор ставки;
- списание ставки;
- анимация барабанов;
- расчёт выигрыша;
- подсветка выигрышных линий;
- звуки spin / reel stop / win / jackpot / lose;
- top-right balance button для перехода на `BALANCE`.

Текущий режим игры настроен как server-required spin: устройство запрашивает
результат spin у backend и затем проигрывает анимацию до полученного результата.

## Backend и API

Контракт API описан в [docs/server-api.openapi.yaml](docs/server-api.openapi.yaml).

Основные endpoint'ы:

- `GET /api/v1/health`
- `POST /api/v1/devices/auth`
- `GET /api/v1/devices/{deviceId}/accounts`
- `GET /api/v1/devices/{deviceId}/balance`
- `POST /api/v1/games/slot/3-reels/spin`
- `POST /api/v1/games/slot/5-reels/spin`
- `POST /api/v1/devices/events`

В папке [backend](backend) есть mock backend на `aiohttp`. Он хранит данные в
памяти, умеет отдавать аккаунты, авторизовать устройство, считать баланс и
генерировать результаты spin.

Запуск mock backend:

```bash
python3 -m pip install -r backend/requirements.txt
python3 backend/mock_server.py --host 0.0.0.0 --port 8080
```

Также подготовлены файлы для запуска backend как systemd service:

- [backend/install_systemd_service.sh](backend/install_systemd_service.sh)
- [backend/iot-terminal-backend.service](backend/iot-terminal-backend.service)

В `backend` также лежат HTML-панели администратора и пользователя. Они относятся
к полной backend-части и могут требовать маршруты, которых нет в локальном
mock-сервере.

## Архитектура прошивки

| Модуль | Назначение |
| --- | --- |
| `DisplayHAL` | TFT, touch, sleep/display hooks, backlight hook |
| `SimpleUI` | лёгкие UI-примитивы и быстрые прямоугольные controls |
| `AppScreens` | экраны, `AppState`, навигация, dirty redraw, power flow |
| `OnScreenKeyboard` | экранная клавиатура для SSID/password |
| `WiFiService` | неблокирующий Wi-Fi scan/connect state machine |
| `WiFiProfiles` | сохранённые Wi-Fi credentials и built-in fallback сети |
| `TimeService` | NTP-время для `HOME` |
| `Pcf8574Buttons` | polling, debounce, short/long POWER |
| `BuzzerService` | неблокирующие звуковые паттерны |
| `SlotGame` | символы, paylines, результат и оценка spin |
| `ServerApiService` | HTTP-клиент для backend API |
| `AppIntegration` | безопасный API для внешней логики |

Главная точка входа: [iot_terminal_ui.ino](iot_terminal_ui.ino).

## Сборка

Подробная инструкция находится в [docs/BUILD.md](docs/BUILD.md).

Кратко:

- Board: `NodeMCU 1.0 (ESP-12E Module)`
- FQBN: `esp8266:esp8266:nodemcuv2`
- CPU Frequency: `80 MHz`
- MMU: `16KB cache + 48KB IRAM (IRAM)`
- Display SPI: `40000000`

Зависимости:

- `ESP8266 core for Arduino 3.1.2`
- `TFT_eSPI 2.5.43`
- `XPT2046_Touchscreen 1.4`

Сборка через `arduino-cli`:

```bash
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .
```

Загрузка:

```bash
arduino-cli upload --fqbn esp8266:esp8266:nodemcuv2 -p <serial-port> .
```

В Arduino IDE рекомендуется поставить `Upload Speed` минимум `230400`.
Если плата и кабель стабильны, можно попробовать `460800`.

## TFT_eSPI setup

Настройка `TFT_eSPI` хранится внутри проекта:

- [iot_terminal_ui.ino.globals.h](iot_terminal_ui.ino.globals.h)

Глобальные файлы библиотеки `TFT_eSPI` редактировать не нужно.

Текущие важные значения:

```cpp
#define ST7789_DRIVER
#define SPI_FREQUENCY 40000000
#define SPI_TOUCH_FREQUENCY 2500000
```

`40000000` выбран как стабильная частота: прежние `80000000` давали артефакты
на реальном дисплее.

## Оптимизация и стабильность

Что было сделано под ограничения ESP8266:

- partial redraw / dirty regions;
- упрощённый fast UI style без дорогих теней и лишнего overdraw;
- SPI дисплея снижен до стабильных `40 MHz`;
- MMU `16KB cache + 48KB IRAM`, что снизило IRAM примерно с `94%` до `70%`;
- touch не опрашивается в sleep/standby;
- Serial logging отключён из-за использования RX/TX под I2C;
- Wi-Fi flow работает через polling/state machine без blocking `while`;
- backend-запросы изолированы в `ServerApiService`.

## Документация

- [docs/BUILD.md](docs/BUILD.md) - сборка и настройки платы.
- [docs/DEPENDENCIES.md](docs/DEPENDENCIES.md) - зависимости проекта.
- [docs/TFT_SPI_SETUP.md](docs/TFT_SPI_SETUP.md) - настройка TFT/SPI.
- [docs/INTEGRATION.md](docs/INTEGRATION.md) - как подключать внешнюю логику.
- [docs/server-api.openapi.yaml](docs/server-api.openapi.yaml) - контракт backend API.
- [docs/Отчёт_Носов_Рублёв_Казакова.pdf](docs/Отчёт_Носов_Рублёв_Казакова.pdf) - финальный отчёт по проекту.
- [backend/README.md](backend/README.md) - запуск mock backend.

## Ограничения и TODO

- Подсветка дисплея пока не управляется аппаратно. Для настоящего гашения нужен отдельный транзистор/PWM.
- `TOP UP` на устройстве пока является заготовкой.
- Mock backend хранит данные в памяти, без постоянной базы.
- HTML-панели backend могут требовать полный backend с дополнительными `/api/v1/backend/...` маршрутами.
- `AppScreens.cpp` стал крупным модулем; в будущем его стоит разделить на несколько файлов.
- Перед публичной публикацией стоит проверить built-in Wi-Fi сети в `WiFiProfiles.cpp`.

## Статус

Проект находится в рабочем демонстрационном состоянии: устройство собирается,
показывает UI, подключается к Wi-Fi, запускает игру, работает с балансом и
может получать результаты spin с backend.
