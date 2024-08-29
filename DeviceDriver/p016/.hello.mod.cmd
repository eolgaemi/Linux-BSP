cmd_/home/ubuntu/pi_bsp/drivers/p016/hello.mod := printf '%s\n'   hello.o | awk '!x[$$0]++ { print("/home/ubuntu/pi_bsp/drivers/p016/"$$0) }' > /home/ubuntu/pi_bsp/drivers/p016/hello.mod
