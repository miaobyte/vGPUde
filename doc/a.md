定义trietree的每个node的子node为N=8 

type struct trienode{ 
    //meta 
    
    uint8 bitmap//bitmap，标记子node[i]是否真实存在 
    uint8 objlist_len //一组对象放在一个node上，而对象的size必须大于这个trienode的管理区域的1/N，否则应该递归的放置在这个node的子node区域 
    uint32*objlist_len 
    objlist //动态数组，objlist_len最大不超过N 
    uint32 dataareasize//区域的大小 
    
    //data区，包含子node或obj数据 ... ... obj区，或子node区 
} 

type struct pool{ 
    const minnodeobj=64*8 
    uint64 poolsize=8 
    trienode* root 
    uint8 levelmax 
    uint32*levelmax 
    levelfirsttrienode_list//每个level的首个空闲指针，用于下次分配对应level的地址。 
}


我把这套内存管理模型，称作立方盒装载法

一段内存空间，可以看作一个正立方体盒子

正立盒内部的空间，可以拆分为8个子正立盒，依次拆分

装载时，根据对象大小，判断装进多大（level）的盒子，然后寻找盒子的位置

 
 init_mem_pool时，需要在pool建立一块sizeof(pool)的区域存储pool信息，并初始化rootnode 
 
 malloc时，需要根据size大小，计算level 根据pool的levelfirsttrienode指针，找到应该放置的node， 然后再在node的动态数组+1，修改node的bitmap，写入对象，注意，在对象的首地址前4个字节，需要写入对象所属的trienode地址，便于通过对象pr，凡推其所属的treinode。
 然后，维护这个level的firsttrienode指针，如果所属node还有大于一个子node的剩余内存，则firsttrienode不变。否则firsttrienode置为无效，等待下次malloc重新查找合适的firsttrienode