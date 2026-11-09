# kernelwatch

A small Waybar module, written in plain C, that shows the latest stable Linux
kernel version from kernel.org and tells you when you are behind or when a new
release drops.

```
 7.3.4
```

## Behavior

- Fetches `https://www.kernel.org/finger_banner` over TLS (OpenSSL, with peer
  and hostname verification) and extracts the latest stable version.
- Compares it with the running kernel (`uname(2)`) and sets the `uptodate` or
  `outdated` CSS class.
- Keeps the last seen version in `$XDG_STATE_HOME/kernelwatch/last`. When the
  stable version changes, it adds the `new` class and sends a desktop
  notification.
- Fails quietly: if the network is down, it prints the cached version with the
  `stale` class. With no cache, it prints `?` with the `error` class.
- A 20 second watchdog guarantees it always prints one line of JSON, so a hung
  connection cannot freeze the bar.
- Prints exactly one line of Waybar JSON on stdout. Diagnostics go to stderr.

## Requirements

- A C compiler (`gcc`), `make`
- OpenSSL development files (`libssl`, `libcrypto`)
- `notify-send` and a running notification daemon, for new release alerts
- A [Nerd Font](https://www.nerdfonts.com/), for the Tux icon
- Optional: `checkpatch.pl` from the Linux kernel tree, for `make check`

## Build and install

```
make
make install
```

`make install` puts the binary in `~/.local/bin` by default. Override the
location with `PREFIX`:

```
make install PREFIX=/usr/local
```

Other targets:

| Target       | Does                                              |
|--------------|---------------------------------------------------|
| `make`       | Build `kernelwatch`                               |
| `make check` | Run `checkpatch.pl` on `src/*.c` and `include/*.h`|
| `make clean` | Remove objects, dependency files and the binary   |

Extra compiler flags can be passed with `EXTRA_CFLAGS`, for example
`make EXTRA_CFLAGS=-fsanitize=address,undefined`.

## Waybar setup

Add the module to `config.jsonc` and list it in `modules-right`:

```jsonc
"custom/kernel": {
	"exec": "kernelwatch",
	"return-type": "json",
	"interval": 1800
}
```

An interval of 15 to 60 minutes is polite to kernel.org.

Style it with the CSS classes below:

| Class      | Meaning                                              |
|------------|------------------------------------------------------|
| `uptodate` | Running kernel equals the latest stable              |
| `outdated` | A newer stable kernel exists                         |
| `new`      | The latest stable changed since the last run         |
| `stale`    | Offline, showing the cached version                  |
| `error`    | Fetch failed and no cached version is available      |

Example:

```css
#custom-kernel.uptodate { border-bottom: 2px solid #a6e3a1; }
#custom-kernel.outdated { border-bottom: 2px solid #f9e2af; }
#custom-kernel.new      { border-bottom: 2px solid #cba6f7; }
#custom-kernel.stale    { border-bottom: 2px solid #6c7086; }
#custom-kernel.error    { border-bottom: 2px solid #f38ba8; }
```

The tooltip shows the running and the latest stable version.

## Project layout

| File                     | Purpose                                         |
|--------------------------|-------------------------------------------------|
| `src/version.c`          | Reads the running kernel version                |
| `src/net.c`              | Fetches the banner from kernel.org over TLS     |
| `src/parser.c`           | Extracts the latest stable version              |
| `src/state.c`            | Reads and writes the last seen version          |
| `src/notify.c`           | Sends a notification for a new release          |
| `src/main.c`             | Ties it together and prints the Waybar JSON     |

## Not implemented yet

- Conditional GET (`If-Modified-Since` / `ETag`) to make polling cheaper.

## License

MIT, see [LICENSE](LICENSE).
