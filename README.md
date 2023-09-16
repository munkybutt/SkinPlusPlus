<h1 align="center">
<!--   <br>
  <a href="http://www.amitmerchant.com/electron-markdownify"><img src="https://raw.githubusercontent.com/amitmerchant1990/electron-markdownify/master/app/img/markdownify.png" alt="SkinPlusPlus" width="200"></a>
  <br> -->
  SkinPlusPlus
  <br>
</h1>

<h4 align="center">Python bindings written in c++, for working with skin data in DCC's</a>.</h4>

<p align="center">
  <a href="https://github.com/munkybutt/SkinPlusPlus/blob/main/LICENSE">
    <img src="https://img.shields.io/github/license/munkybutt/SkinPlusPlus?style=for-the-badge"
  </a>
  <a href="https://github.com/munkybutt/SkinPlusPlus/releases/tag/v0.3.0">
    <!-- <img src="https://badge.fury.io/gh/munkybutt%2FSkinPlusPlus.svg?style=for-the-badge"> -->
    <img src="https://img.shields.io/github/release/munkybutt/SkinPlusPlus?style=for-the-badge&include_prereleases">
  </a>
  <a href="https://saythanks.io/to/munkybutt">
      <img src="https://img.shields.io/badge/Say%20Thanks-!-1EAEDB.svg?style=for-the-badge">
  </a>
  <a href="https://www.paypal.me/munkybuttballs">
    <img src="https://img.shields.io/badge/$-donate-ff69b4.svg?maxAge=2592000&amp;style=for-the-badge">
  </a>
</p>

<p align="center">
  <a href="#disclaimer">Disclaimer</a> •
  <a href="#key-features">Key Features</a> •
  <a href="#usage">Usage</a> •
  <a href="#performance">Performance</a> •
  <a href="#how-to-compile">How To Compile</a> •
  <a href="#roadmap">Roadmap</a> •
  <a href="#personal-info">Personal Info</a>
</p>


## Disclaimer

This software (SkinPlusPlus) is provided "as is", without any warranty. The authors or copyright holders will not be liable for any damages or claims related to the software or its use.


## Key Features
* Save and load skin data with speed
  - Logic is written in c++ but exposed with python bindings

* Work directly with numpy ndarrays
  - Pybind11 is used to generate the python bindings, so the API uses numpy ndarrays for optimal performance

* Currently supported DCCs:
  - 3DsMax:
    - 2024 - Python310
    - 2023 - Python39
    - 2022 - Python37
  	- 2021 - Python37
  - Maya:
    - 2023 - Python39
    - 2022 - Python37

## Usage
There are three types of data that are of interest when working with skin data:
- vertex bone weights (float)
- vertex bone ids (int)
- vertex positions (float)

PySkinData is a c++ struct containing the above data, exposed to python as the SkinData class.  
This struct allows the data to be typed correctly rather than all typed as floats.  
On the c++ side, the data is stored in Eigen matrices.  
On the Python side, the data is exposed via Pybind11 as numpy arrays.

Due to the relationship between Eigen and Pybind11, there is no performance hit when passing arrays to and from c++.

🔥 It is fast 🔥

It also provides a simple interface to the raw data in the form of the following properties:
- SkinData.weights
- SkinData.bone_ids
- SkinData.positions

## Performance
Performance benchmarks are done on a mesh with 507,906 vertices, each with 6 influences.

### 3DsMax 2022
#### Get Skin Data

| Method                                      | Time-secs | x Faster  | % Faster  |
|:--------------------------------------------|-----------|-----------|-----------|
| pymxs -> numpy array                        | 68.231    | base      | base      |
| maxscript -> numpy array                    | 47.859    | 001.426x  | 00142.56% |
| SDK function publish -> numpy array         | 12.800    | 005.330x  | 00533.06% |
| skin_plus_plus -> numpy array               | 00.335    | 203.631x  | 20363.10% |

#### Set Skin Data

| Function                          | Time-secs | x Faster | % Faster |
|:----------------------------------|-----------|----------|----------|
| pymxs <- mxs.Array                | 10.008    | base     | base     |
| maxscript <- mxs.Array            | 08.258    | 01.211x  | 0121.18% |
| SDK function publish <- mxs.Array | 07.368    | 01.358x  | 0135.82% |
| SDK struct primative <- mxs.Array | 05.838    | 01.714x  | 0171.43% |
| SDK primative <- mxs.Array        | 05.694    | 01.757x  | 0175.76% |
| skin_plus_plus <- np.ndarray      | 00.184    | 54.345x  | 5434.57% |

### Maya 2022
#### Get Skin Data

| Method                        | Time-secs | x Faster  | % Faster  |
|:------------------------------|-----------|-----------|-----------|
| maya.cmds -> numpy array      | 81.572    | base      | base      |
| pymel -> numpy array          | 04.647    | 017.550x  | 01755.08% |
| skin_plus_plus -> numpy array | 0.5235    | 155.810x  | 15580.99% |

## How To Compile
### Dependencies
The c++ backend has 3 main dependencies
- Pybind11
  - Exposes c++ logic to python, circumventing native SDK mechanisms
- Eigen
  - Matrix library for c++, provides extremely performant backend for numpy
- fmt
  - Allows for easy string formatting in c++

## Roadmap
* Include Vertex Positions
  - [X] This will enable features for applying skin data to different topologies

* Support multiple DCCs
  - [x] 3DsMax
  - [x] Maya
  - [ ] Blender
  - [ ] MotionBuilder
  - [ ] Houdini

* Send data between DCCs
  - [x] Make skin data serialisable 
  - [ ] Pass data between DCC using local servers
  - This will provide a foundation for a DCC agnostic skinning pipeline

* Expand toolkit to include functions other than get and set.

## Personal Info
> Webbie [techanimdad.com](https://techanimdad.com) &nbsp;&middot;&nbsp;
> GitHub [@munkybutt](https://github.com/munkybutt)
