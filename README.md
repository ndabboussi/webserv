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



Feuile de route :
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

