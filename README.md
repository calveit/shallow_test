# Data oriented experiment

Simple game demonstrating data oriented approach to gameplay. 

#### Gameplay
Several countries fight for territory. Each of the country is represented by a thick dot (hub), which moves around and spawn armies.
Game world is divided into provinces, which can be controlled by a country. Hub spawn ratio depends on the size of controlled territory. 
The more territory a country control, the faster it spawns new armies. Armies move around, take control of territory and kill armies of other countries. 
Country and armies movement is controlled by a dynamic vector field controlled by trigonometric functions.

Several screenshots: 
* [Armies](/img/armies.png)
* [Vector field controlling the movement](/img/gradient.png)
* [Province ownership](/img/ownership.png)
* [Killing armies with mouse cursor (RMB)](/img/kill.png)
* [Pushing armies with mouse cursor (LMB)](/img/push.png)


#### Frame organization
TODO

Profiling snapshot (made with Optick) for 15 countries, 2304 provinces, 1 million armies. 
Average frame time: 46.247 ms on XPS 15 9570 (Intel Core i7-8750H, 16 GB RAM, Windows 10 Home).

![Frame organization](/img/profile_overview.png)
![Frame lengths](/img/profiling_times.png)
