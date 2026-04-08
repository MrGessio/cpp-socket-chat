TCP chat server written in C++ using sockets and multithreading.

Features:
- public messages visible to all users,
- private messages (/msg),
- nickname change (/nick),
- list of online users (/list).
  


Build:
      ```
      g++ server.cpp -o server -pthread      
      g++ client.cpp -o client -pthread
      ```
      
Run:
    ```
      ./server
      ./client
    ```

Commands:
  ```
/nick <name>            change your nickname
/msg <user> <message>   send a private message
/list                   show all online users
  ```
