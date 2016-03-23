## userA给userB发文件的过程如下：


#### userA发送文件传输请求：

userA@@@filename|size&&&FileSend>>>userB

#### 服务器转发请求给userB
userB@@@filename|size&&&FileSend<<<userA


#### userB回应：

userB@@@0&&&AskFileData>>>userA    ```允许发送0号分组

userB@@@NO&&&AskFileData>>>userA   ```拒绝接收文件


#### 服务器转发给userA:

userA@@@0&&&AskFileData<<<userB  ```请求发送0号分组

userA@@@NO&&&AskFileData<<<userB   ```拒绝接收文件

#### userA开始发送文件

userA@@@DataData&&&FileData>>>userB   ```开始发文件分组

#### 服务器转发分组给userB

userB@@@DataData&&&FileData<<<userA
