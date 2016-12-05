# HyperspectralFiltering

## **Set-up**
#### Step 1) OpenCV Template Project
There are many great guides online for installing OpenCV and setting up an OpenCV project, so we will not cover that here.
Everyone's OpenCV setup can differ, so we've left that out of our project for simplicity. Our project is tested using 
OpenCV 3.1.0, on x64 architecture. 
### Step 2) Clone and Move Files
Clone the files in the project by your preffered method. Move these files to the source of your project. Using a blank OpenCV
 Visual Studio project, this means moving the files to ```ProjectName\ProjectName\```. Be sure the code is added to ```Source Files```
 on your project.
### Step 3) Acquire SpectralImages
Using the Hyperion Satteliete data, find a spectral image set to use. The project works exclusively with the GeoTIFF format.
We have some example image sets you can find on our [Google Drive] (https://drive.google.com/drive/folders/0B1o0P-AAbXBUZ3MxTHZERTBXcjg?usp=sharing). 
Once you have a data set to use, move it to the same place your source files have moved. Using a blank OpenCV Visual Studio 
project, this means moving the files to ```ProjectName\ProjectName\```. **Do not rename the folder or images**. Using the example
 Seattle ImageSet, move the folder inside of ```Seattle.rar``` (named ```EO1H0460272003133110PW```) to ```ProjectName\ProjectName\``` such that the
 images reside in ```ProjectName\ProjectName\EO1H0460272003133110PW```.
### Step 4) Customize the project
You're almost ready to run the code. In ```main.cpp``` find where you create a SpecImage object. It might look like this: 
```SpecImage newImg("EO1H0010492002110110KZ_1T");```. Replace the quoted string to be the same as the folder name of your image set.
For example, using the Seattle image set, your SpecImage object should look like this: ```SpecImage newImg("EO1H0460272003133110PW");```.
### Step 5) Ready!
You are now ready to run the project! Use the example methods in main.cpp to understand the basic capabilities of the project. 
Images are returned as OpenCV Mat objects, so you can utalize any OpenCV methods to manipulate the data.
