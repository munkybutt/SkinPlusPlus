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
  <a href="https://github.com/munkybutt/SkinPlusPlus/releases/tag/v0.1.0">
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
  <a href="#performance">Performance</a>
</p>


## Key Features
* Save and load skin data with speed
  - Logic is written in c++ but exposed with python bindings

* Work directly with numpy ndarrays
  - The python bindings accept and return numpy ndarrays for optimal performance

* Currently supported DCCs:
  - 3DsMax:
  	- Provided bindings are for 2022, but the backend should be compatible with any version of 3DsMax that has python

## Performance
Performance tests are done on a mesh with 507,906 vertices, each with 6 influences.

3DsMax 2022
| Method                                      | Time in seconds     | x Faster             | % Faster             |
|---------------------------------------------|---------------------|----------------------|----------------------|
| pymxs -> list                               | 20.34769090000009   | base line            | base line            |
| maxscript -> numpy array                    | 15.51825759999997   | 1.3112097649416599 x | 131.12097649416597 % |
| maxscript -> list                           | 14.42323169999986   | 1.4107580966060669 x | 141.0758096606067 %  |
| SDK primative -> list                       | 7.435437399999955   | 2.7365829076847867 x | 273.65829076847865 % |
| SDK function publish -> list                | 6.338866400000143   | 3.2099889185232917 x | 320.99889185232917 % |
| SDK struct primative -> list                | 5.98266609999996    | 3.4011075597216136 x | 340.11075597216137 % |
| pybind11 automatic -> numpy array           | 1.2681291999999758  | 16.045439928360988 x | 1604.5439928360988 % |
| pybind11 move -> numpy array                | 1.09791139999993    | 18.533090101807293 x | 1853.3090101807293 % |
| pybind11 copy -> numpy array                | 0.9864563000000999  | 20.627057579740764 x | 2062.7057579740763 % |
| pybind11 -> list                            | 0.9028401000000486  | 22.537424844110262 x | 2253.7424844110265 % |
| pybind11 reference_internal -> numpy array  | 0.4243109000001368  | 47.954674037347445 x | 4795.467403734745 %  |
| pybind11 automatic_reference -> numpy array | 0.4236172999999326  | 48.03319151508526 x  | 4803.3191515085255 % |
| pybind11 take_ownership -> numpy array      | 0.41753419999986363 | 48.73299217167536 x  | 4873.299217167536 %  |
| pybind11 reference -> numpy array           | 0.41740709999999126 | 48.747831313843285 x | 4874.783131384329 %  |
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
