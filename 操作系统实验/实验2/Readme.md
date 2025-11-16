# 实验2  
## 2.1软中断  
实验前准备，使用man命令查看各系统调用帮助手册  
fork  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/a4da7078-7686-46f0-b743-0ca029a401c8" />  
kill  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/717c32d9-5af0-48c3-b777-f3a7874c6b37" />  
exit  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/088bd6a8-ebf1-47dc-adbd-0287c7addddf" />  
signal  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/be9fbb4b-7c07-4c7c-9aa3-212e273662fd" />  
sleep  
<img width="1734" height="927" alt="image" src="https://github.com/user-attachments/assets/0e5b6588-5ac8-4382-843c-0d1afbc6b0b7" />  

补全程序运行：  
等待5s后    
<img width="840" height="675" alt="image" src="https://github.com/user-attachments/assets/89c33b32-ebae-40b3-bac2-fd1995bd33a3" />  
按下ctrl+c或\  
<img width="859" height="680" alt="image" src="https://github.com/user-attachments/assets/3a60410f-367b-43a3-ad4d-0d0982318b6d" />  
<img width="841" height="680" alt="image" src="https://github.com/user-attachments/assets/997ba255-a734-4668-9e93-68fad8452e86" />  
输出顺序不固定

改为闹钟中断  
<img width="838" height="450" alt="image" src="https://github.com/user-attachments/assets/f58e3756-816e-441b-820c-f15c10a8f5d6" />  
<img width="840" height="677" alt="image" src="https://github.com/user-attachments/assets/4eb4d4d0-5e25-4e0f-8d58-49cbc26b2dcd" />  
没有明显区别

## 2.2管道通信  
有锁情况  
<img width="1734" height="1095" alt="image" src="https://github.com/user-attachments/assets/31eb7464-c19b-4ed7-ab69-91d6079434ec" />    

去掉锁  
<img width="1734" height="1095" alt="image" src="https://github.com/user-attachments/assets/e803ae5c-fca0-4917-9d91-65c7d8d32308" />  


## 2.3 内存分配  
补全代码，使用3种算法，同时完成   功能 
初始情况如下：  
<img width="738" height="516" alt="image" src="https://github.com/user-attachments/assets/7283e51b-4318-4c0e-b3ac-41a8a18ad0fd" />  

①使用FF算法，连续分配大小为20，35，400，查看资源分配情况  
<img width="738" height="316" alt="image" src="https://github.com/user-attachments/assets/6dc8bb80-758a-4584-a6c8-f947a94ab6e4" />  
<img width="738" height="64" alt="image" src="https://github.com/user-attachments/assets/5ece683d-5303-429d-ad12-336bf2135cd3" />  

释放进程2，观察  
<img width="738" height="360" alt="image" src="https://github.com/user-attachments/assets/f77bde1d-6776-48e4-8e5f-3b8a2850381a" />  

发现空闲区被从前往后分割  

②使用BF，分配100，200，300，如图  
<img width="738" height="315" alt="image" src="https://github.com/user-attachments/assets/48251496-11ef-451c-acfe-da7c43912a42" />  

释放2，放入120<200：  
<img width="738" height="830" alt="image" src="https://github.com/user-attachments/assets/09699e89-0d04-49b8-a1fb-5ea2d04cfc6f" />  

120放到那个最小空闲块里面，而不是后面的更大块。  

③使用WF，分配100，50，300  
<img width="738" height="310" alt="image" src="https://github.com/user-attachments/assets/2172d6ab-6798-4dcb-9cbe-3264a04e6f77" />  

释放1，2  
<img width="738" height="314" alt="image" src="https://github.com/user-attachments/assets/84043449-4289-4a17-ad61-5c3376a23b10" />  


分配大小为80的  
<img width="738" height="817" alt="image" src="https://github.com/user-attachments/assets/77c51df1-2f67-4299-83b3-5cae45b18998" />  
可见WF会选择最大的空闲块，不是100，而是后面的最大空间  
 
④分配100，剩余924，再分配920，发现剩余太少，被合并成一块，程序会把这4当作内碎片加入920中  
<img width="738" height="276" alt="image" src="https://github.com/user-attachments/assets/36ea09e0-ecbe-450f-876b-6b38bdfd7095" />  

释放1  
<img width="738" height="288" alt="image" src="https://github.com/user-attachments/assets/2fabca95-4f7e-4d04-9f23-a45be99560be" />    

⑤分配200，300，200，情况如下  
<img width="738" height="318" alt="image" src="https://github.com/user-attachments/assets/29b4cbf9-551a-4102-8c73-66297d6795b8" />  

杀死进程2，大小300  
<img width="738" height="318" alt="image" src="https://github.com/user-attachments/assets/c23518b5-b6fd-4301-a869-cfc253773fc4" />  

分配大小500，没有任何一块满足，合并所有空闲碎片形成一个大块，再分配给进程  
<img width="738" height="330" alt="image" src="https://github.com/user-attachments/assets/7d131e4c-7d3e-4894-927e-0cc3bf0ee26d" />  

⑥分配100，200，150：  
<img width="738" height="311" alt="image" src="https://github.com/user-attachments/assets/81cd32eb-144d-4415-b51d-10611878ad8d" />  

杀死进程1，2，两块相邻空闲合并为一块  
<img width="738" height="294" alt="image" src="https://github.com/user-attachments/assets/db1b546d-32ba-43d6-b851-4a19b5308b0d" />  












算法的思想和特点，提高性能：  

内外碎片：  

合并空闲块方法：  
