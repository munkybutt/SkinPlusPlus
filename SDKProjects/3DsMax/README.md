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

### 3DsMax 2022
#### Get Skin Data

| Method                                      | Time-secs | x Faster | % Faster |
|:--------------------------------------------|-----------|----------|----------|
| pymxs -> list                               | 20.347    | base     | base     |
| maxscript -> numpy array                    | 15.518    | 01.311x  | 0131.12% |
| maxscript -> list                           | 14.423    | 01.410x  | 0141.07% |
| SDK primative -> list                       | 07.435    | 02.736x  | 0273.65% |
| SDK function publish -> list                | 06.338    | 03.209x  | 0320.99% |
| SDK struct primative -> list                | 05.982    | 03.401x  | 0340.11% |
| pybind11 automatic -> numpy array           | 01.268    | 16.045x  | 1604.54% |
| pybind11 move -> numpy array                | 01.097    | 18.533x  | 1853.30% |
| pybind11 copy -> numpy array                | 00.986    | 20.627x  | 2062.70% |
| pybind11 -> list                            | 00.902    | 22.537x  | 2253.74% |
| pybind11 reference_internal -> numpy array  | 00.424    | 47.954x  | 4795.46% |
| pybind11 automatic_reference -> numpy array | 00.423    | 48.033x  | 4803.31% |
| pybind11 take_ownership -> numpy array      | 00.417    | 48.732x  | 4873.29% |
| pybind11 reference -> numpy array           | 00.417    | 48.747x  | 4874.78% |

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
