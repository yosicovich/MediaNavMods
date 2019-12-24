# MediaNavMods
Famous medianav.ru mod EvoTech source code. Recovered to the state as on 18.06.2019 after ssd crash.

This repository contains famous EvoTech mod for MediaNav Evolution Head unit set into many Renault/Vaz/others cars.
Russian speaking forum is located at forum.medianav.ru. This source code is not complete since complete source code has been lost after ssd drive crash(Very thank you SAMSUNG for this!). 
This is the latest backup available. It was done at 18.06.2019. It is the very beginning of EvoTech development but it contains complete code for video player since it was not modified and many ideas behind EvoTech itself. 
This code can be used for education purposes or re-used for other Windows CE based projects as long as this projects are not commercial, see license.

The source code contains a bunch of third-party code. We've tried to preserve all licenses possible but it can still be incomplete.
If respective owners of this code feel that their rights are broken please email to webmaster@medianav.ru with you concerns and fixes are to be applied on your request.

## Repository structure
1. **build** folder contains build scripts.
2. **ce** folder contains sources that are targeted to Head Unit execution.
3. **pc** folder contains sources that are targeted to PC execution such as patching tool *import-patcher* and *lgutool*.
4. **ce/au1300sdk** is Medianav's core platform SDK packet that is used during builds.
5. **ce/medianav/thirdparty** is third-party code fixed to be build-able for medianav SDK. see above.
6. **ce/medianav/mods** - main mods code.

## How to build

### Pre-requires
1. Visual studio 2008
2. Windows CE 6.0 R3 platform builder with latest publicity available updates.
3. au1300 platform SDK that is at **ce/au1300sdk**

* Checkout repository. Correct **ce\medianav\mods\mediasdk.vsprops** accordingly(set correct path to installed Media part of SDK).
* Open **mods.sln**.
* Build.
