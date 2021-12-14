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
  <a href="https://github.com/munkybutt/SkinPlusPlus/releases/tag/v0.2.0">
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
  <a href="#key-features">Key Features</a> •
  <a href="#performance">Performance</a> •
  <a href="#roadmap">Roadmap</a>
</p>


## Key Features
* Save and load skin data with speed
  - Logic is written in c++ but exposed with python bindings

* Work directly with numpy ndarrays
  - Pybind11 is used to generate the python bindings, so the API uses numpy ndarrays for optimal performance

* Currently supported DCCs:
  - 3DsMax:
  	- Provided bindings are for 2022, but the backend should be compatible with any version of 3DsMax that has python

## Performance
Performance tests are done on a mesh with 507,906 vertices, each with 6 influences.
Data returned is:
- numpy array of vertex weights
- numpy array of vertex bone ids
- numpy array of vertex positions

### 3DsMax 2022
#### Get Skin Data

| Method                                      | Time-secs | x Faster  | % Faster  |
|:--------------------------------------------|-----------|-----------|-----------|
| pymxs -> numpy array                        | 68.231    | base      | base      |
| maxscript -> numpy array                    | 47.859    | 001.426x  | 00142.56% |
| SDK function publish -> numpy array         | 12.800    | 005.330x  | 00533.06% |
| pybind11 -> numpy array                     | 00.335    | 203.631x  | 20363.10% |

#### Set Skin Data

| Function                          | Time-secs | x Faster | % Faster |
|:----------------------------------|-----------|----------|----------|
| pymxs <- mxs.Array                | 10.008    | base     | base     |
| maxscript <- mxs.Array            | 08.258    | 01.211x  | 0121.18% |
| SDK function publish <- mxs.Array | 07.368    | 01.358x  | 0135.82% |
| SDK struct primative <- mxs.Array | 05.838    | 01.714x  | 0171.43% |
| SDK primative <- mxs.Array        | 05.694    | 01.757x  | 0175.76% |
| pybind11 <- np.ndarray            | 00.184    | 54.345x  | 5434.57% |

## Roadmap
* Include Vertex Positions
  - This will enable features for applying skin data to different topologies
  - DONE

* Support multiple DCCs
  - Maya
  - Blender
  - MotionBuilder
  - Houdini

* Send data between DCCs
  - Serialise skin data and pass between DCC using local servers
  - This will provide a foundation for a DCC agnostic skinning pipeline

* Expand toolkit to include functions other than get and set.

<!--## 
## How To Use

Support
<a href="https://www.buymeacoffee.com/5Zn8Xh3l9" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/purple_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>

<p>Or</p> 

<a href="https://www.patreon.com/amitmerchant">
	<img src="https://c5.patreon.com/external/logo/become_a_patron_button@2x.png" width="160">
</a>

-->

## Personal Info
> Webbie [techanimdad.com](https://techanimdad.com) &nbsp;&middot;&nbsp;
> GitHub [@munkybutt](https://github.com/munkybutt)
