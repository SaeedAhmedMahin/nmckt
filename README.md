# nmckt
Chat application made with C and Ncurses (Socket programming) 

## Requirements Summary 
1. Private messaging
2. Global Messages to all users
3. Message history is saved
4. Ncurses for TUI (users should have different colors in global chat)
5. Notifications when users join or leave.
6. Color customization per user or group

# Required Features

1. Server
● Accept multiple clients (threading is done in template, just understand
where to edit).
● Maintain active users list.
● Maintain chat history:
○ Each message stored as a string with:
■ Type: group/private
■ Sender
■ Recipient(s)
■ Content

● Broadcast messages based on selection:
○ Group messages: All users
○ Private messages: Target user only
● Must ensure that connected client has a unique username, no client
username should match.
● Save chat history to a file using redirection so that even if computer restarts
old messages can be retrieved via client username.
2. Client
● ncurses interface with three windows:
1. Active Users/Groups Window: Scrollable list of users and groups.
2. Chat History Window: Shows messages filtered by selected user or
broadcast (all user group).
3. Input Window: Type messages selecting a user or group.
● Selecting a user/group updates the chat history window to show relevant
messages.

● Typing a message while a user/group is selected sends it to the selected
target(s).
● Color-coded usernames and message types.

3. Message Handling
● Messages are plain strings (no structs). Chat history displays message
type, sender, target (if any), and content:
● Timestamps for messages. Show messages sorted by message time.
4. ncurses Interface
● Active Users Window: Lists all connected users.
● Chat History Window: Scrollable display of messages with details.
● Input Window: User types messages
● Color Coding: Different colors for usernames and message types.


# Simplification 

● No structs required — use arrays of strings/pointers for users, groups,
and chat history.
● No FILE I/O functions — test input using < operator.
● Multiple clients can run in separate terminal windows on the same machine
(via localhost).
