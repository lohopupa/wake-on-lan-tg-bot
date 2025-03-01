# wake-on-lan-tg-bot

**wake-on-lan-tg-bot** is a simple Telegram bot written in **C** to trigger **Wake On LAN (WOL)** on devices connected to an **OpenWRT** router.

## Build

To build the project, run:

```
./build.sh
```

The compiled binary will be placed in the **`./build`** directory.

---

## Running Locally

1. To run the bot locally, you need to export the following environment variables:

```bash
export TELEGRAM_BOT_TOKEN=""
export TARGET_IP=""
export TARGET_MAC=""
export WOL_COMMAND="etherwake -i br-lan %s"
export CHECK_STATUS_COMMAND="ping -c 1 -W 1 %s > /dev/null 2>&1"
```

2. Run execuatble from build folder
```bash
./build/wol-bot-x86_64
```

---

## Deploying to OpenWRT Router

1. Copy the compiled binary to the router:

```bash
scp ./build/wol-bot-arm64 router:/usr/bin/wol-bot
```

2. Edit environment variables in `wol-bot.example` to match your setup.
3. Copy the `wol-bot.example` file to the router:

```bash
scp wol-bot.example router:/etc/init.d/wol-bot
```

4. Set executable permissions on the init script (if needed):

```bash
ssh router "chmod +x /etc/init.d/wol-bot"
```

---

## Service Management

Once deployed, you can manage the bot as a service:

- Start:  
    ```service wol-bot start```
- Stop:  
    ```service wol-bot stop```
- Restart:  
    ```service wol-bot restart```

---

## License

This project is licensed under the **MIT License**.