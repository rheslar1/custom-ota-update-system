# Embedded Linux OTA Path

The Linux path models an A/B root filesystem update for an edge gateway:

- active slot: `rootfs_a`
- inactive slot: `rootfs_b`
- artifact type: signed RAUC/Mender-style rootfs bundle
- transport: HTTPS
- verification: manifest signature plus SHA-256 digest
- recovery: automatic rollback to the previous confirmed slot after failed trial boots

## Systemd Integration Sketch

```ini
[Unit]
Description=Custom OTA Agent
After=network-online.target

[Service]
ExecStart=/usr/bin/custom-ota-agent --manifest https://updates.example.com/bems/channel.json
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

The host C++ model validates the same decision surface before a Linux service downloads or installs a real bundle.
