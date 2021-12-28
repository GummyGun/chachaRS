all:
	gcc chacha.c -lsodium -o chachaRS

test:
	sudo mv chachaRS /usr/bin


.PHONY:clean
clean:
	sudo rm /usr/bin/chachaRS

