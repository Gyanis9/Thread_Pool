**项目名称：** 基于可变参模板实现的线程池  
**Git 地址：** [git@github.com:Gyanis9/Thread_Pool.git]  
**平台与工具：** CLion2024.1, Ubuntu24.04, g++, GDB（用于调试和死锁分析）  
**项目概述：**
- 开发了一个高性能线程池，用于处理高并发网络服务器和后台任务，着重于可扩展性和灵活的任务管理。
- 使用可变参模板和引用折叠原理实现 `submitTask` 接口，支持动态提交具有任意函数签名和参数的任务。
- 集成 `std::future` 以处理任务结果，提供异步任务执行功能。
- 使用 `std::map` 和 `std::queue` 管理线程对象和任务，确保高效的线程与任务管理。

**主要功能：**
1. **灵活的任务提交：** 通过可变参模板，`submitTask` 方法支持不同参数类型的任务提交。
2. **动态线程管理：** 线程池支持 **固定模式** 和 **缓存模式**，根据任务负载动态创建或回收线程。
3. **任务结果处理：** 使用 `std::packaged_task` 包装任务，通过 `std::future` 获取结果，确保非阻塞执行。
4. **线程通信机制：** 通过条件变量（`std::condition_variable`）和互斥锁（`std::mutex`）实现线程与任务队列之间的同步，确保任务正确分配。
5. **平台适配性：** 支持 Windows 和 Linux 平台，解决了 Linux 平台上出现的死锁问题，并使用 GDB 进行调试分析。
6. **可配置参数：** 线程池的参数（如初始线程数、最大线程数、任务队列大小）可通过外部配置文件（INI 格式）进行配置。

**挑战与解决方案：**
- **死锁问题：** 在资源回收和线程退出时遇到死锁问题，特别是在 Linux 平台上。通过 GDB 调试分析，定位到线程退出过程中发生死锁的代码段，并找到了解决方案。
- **平台差异性：** 线程池在 Windows 平台上运行正常，但在 Linux 上出现死锁问题。经过分析，发现是由于平台间线程同步机制的差异，最终通过调整互斥锁和条件变量的使用解决了该问题。

**使用技术：**
- **编程语言与库：** C++，Boost（属性树，INI 解析），GDB（用于调试）
- **并发机制：** 线程、互斥锁、条件变量
- **平台支持：** 跨平台（Windows 和 Linux）
