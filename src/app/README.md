This app, MillenniumSerpent, is the client to the MillenniumEncryption server.  It's a Qt app, with a MainWindow with nine important children:

* The ClientDatabaseManager, a QObject defined in database.h, which maintains tables of messages and friends
* The ClientSocketManager, a QObject defined in clientsocket.h, which handles all communication with the server
* Five QWidgets - the CodeBox (which holds the codebook), the FriendsBox (which displays friends and their online status), the RequestsBox (which displays incoming and outgoing friend requests), the MessagesBox (which shows a conversation), and the LoginWidget (which allows for login and is only visible upon starting the program)
* A QLineEdit and a QPushButton used for writing and sending messages