# webserv

CGI = 
    The Common Gateway Interface, or CGI, is a set of standards that define how information is exchanged between the web server and a custom script.

    The CGI specs are currently maintained by the NCSA and NCSA defines CGI is as follows −

    The Common Gateway Interface, or CGI, is a standard for external gateway programs to interface with information servers such as HTTP servers.

    The current version is CGI/1.1 and CGI/1.2 is under progress.


Fonction	Utilité
fork()	Crée un nouveau processus (enfant) identique au parent. Utilisé pour lancer un CGI sans bloquer le serveur.
execve(path, argv, envp)	Remplace le processus courant par un nouveau programme. En CGI : tu exécutes php-cgi, python, etc.
waitpid(pid, &status, options)	Attends qu’un processus enfant se termine, pour récupérer son code retour et éviter les zombies.
kill(pid, sig)	Envoie un signal à un processus (ex: tuer un CGI trop long).
signal(sig, handler)	Associe un handler à un signal (ex: SIGCHLD pour nettoyer les processus enfants, SIGINT pour arrêter proprement ton serveur).



Feuille de route :
1. se renseigner sur les fichiers de config
2. parser fichier de config
    aa. verifier les brackets, le mot serveur
    a. verifier les erreurs (ex: deux serveurs ouverts sur le meme port)
    b. si dans l'init du serveur y a pas de root, checker si les chemins des locations sont abs et si fonctionnent
3. connecter le serveur avec le navigateur


METHODS
0 aucune
1 GET
2 POST
3 GET+POST
4 DELETE
5 GET+DELETE
6 POST+DELETE
7 GET+POST+DELETE

WEBSERVER: https://www.youtube.com/watch?v=9J1nJOivdyw
API: https://www.youtube.com/watch?v=ByGJQzlzxQg
https://www.youtube.com/watch?v=T0DmHRdtqY0 
Ports: https://www.youtube.com/watch?v=YSl6bordSh8
OSI: https://www.youtube.com/watch?v=26jazyc7VNk



WebServer:
1. listens 
2. on a port 
3. for a request
4. send via a TRansport Protocol
5. return a response
6. through a ressource




Pour webserv: (1) bien comprendre et faire un parsing minimaliste du fichier de config, l idee c est de vraiment autorise juste le necessaire pour respecter le sujet et interdire tout le reste ->  se baser sur nginx (2) faire un serveur basique avec epoll. Pour une introduction au project + une comprehension du serveur et des codes d erreur de base [https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7]. Pour comprendre la programmation concurrentielle [https://www.youtube.com/watch?v=RlM9AfWf1WU ]. Sur epoll, le man avec le code de la base du serveur (attention a bien verifier ready to read ET ready to write dans la main loop, pas comme sur l exemple, on peut en parler plus tard) [https://man7.org/linux/man-pages/man7/epoll.7.html]. Pour epoll in depth [https://medium.com/@avocadi/what-is-epoll-9bbc74272f7c] et [https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642] + les flag pour epoll ctl [https://man7.org/linux/man-pages/man2/epoll_ctl.2.html].

Ensuite il faut comprendre (peut etre meme avant epoll) les sockets non bloquantes: debut [https://dev.to/vivekyadav200988/understanding-blocking-and-non-blocking-sockets-in-c-programming-a-comprehensive-guide-2ien],  le rapport entre epoll et non bloquant [https://eklitzke.org/blocking-io-nonblocking-io-and-epoll], [https://copyconstruct.medium.com/nonblocking-i-o-99948ad7c957] le reste ca va se repeter un peu. Ne pas laisser trop de document te bloquer, tu vas vite comprendre en pratiquant avec des exemples minimalistes + tu pourras construire dessus par la suite.

Ensuite pour les CGI: la rfc cgi [https://datatracker.ietf.org/doc/html/rfc3875], le wiki [https://en.wikipedia.org/wiki/Common_Gateway_Interface], explication plus clair [https://coolicehost.com/billing/knowledgebase/article/101/what-is-a-common-getaway-interface-cgi/] -> tu peux ajouter les cgi a la fin c est vrt pas le coeur du sujet, hesite pas a demander conseil sur comment les implementer, il va falloir fork pour avoir un child et exec le script