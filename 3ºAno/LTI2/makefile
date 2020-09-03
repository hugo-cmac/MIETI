	gcc -Wall gestor.c -lm `mysql_config --cflags --libs`
	gcc -Wall concentrador.c -o concentrador
	gcc -Wall monitor.c -o monitor

	gnome-terminal -- bash -c ./a.out
  sleep 5
	gnome-terminal -- bash -c ./concentrador
