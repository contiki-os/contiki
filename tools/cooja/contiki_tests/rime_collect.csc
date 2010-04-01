<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/native_gateway</project>
  <simulation>
    <title>Rime collect test</title>
    <delaytime>0</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>10000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>67.0</transmitting_range>
      <interference_range>0.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype98</identifier>
      <description>Contiki Mote Type #1</description>
      <contikiapp>[CONTIKI_DIR]/examples/rime/example-collect.c</contikiapp>
      <commands>make example-collect.cooja TARGET=cooja</commands>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Battery</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>38.67566417548448</x>
        <y>47.31532819237484</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>71.13430279192914</x>
        <y>55.964918387262955</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>228.04679204790637</x>
        <y>87.17819808323965</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>3</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>272.42783222170533</x>
        <y>46.64334378879388</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>238.61415527274</x>
        <y>44.41698596888275</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>5</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>132.73939224849255</x>
        <y>69.21851375812221</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>6</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>13.282402591495124</x>
        <y>37.55717734948646</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>7</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>231.24739439405175</x>
        <y>48.67375039920239</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>8</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>207.8959314238542</x>
        <y>1.1350394672889341</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>9</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>92.82161206304569</x>
        <y>92.33145969594939</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>10</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>160.99396124295916</x>
        <y>19.643001828505756</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>11</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>200.78134764559428</x>
        <y>12.892752477526937</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>12</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>205.39914563029964</x>
        <y>28.760487893562114</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>13</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>252.08232300754125</x>
        <y>72.49857017173812</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>14</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>229.71392970623077</x>
        <y>6.54664783066401</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>15</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>278.53902340242763</x>
        <y>68.52057141636107</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>16</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>63.58843478737991</x>
        <y>53.533699264766824</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>17</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>143.25717547901027</x>
        <y>61.23529184398511</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>18</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>238.99233371296435</x>
        <y>11.57402085202307</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>19</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>131.463497184274</x>
        <y>37.91565308310023</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>20</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>299.4799135787668</x>
        <y>55.16132007269603</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>21</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>187.71659571763186</x>
        <y>9.08434815157203</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>22</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>102.203173631275</x>
        <y>62.50474380428127</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>23</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>125.71665361687481</x>
        <y>43.5458073676737</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>24</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>252.63631602446236</x>
        <y>17.060026732849032</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>25</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>266.5666796770194</x>
        <y>8.117217835238177</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>26</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>131.87192517986617</x>
        <y>32.127513593397026</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>27</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>30.652367771559508</x>
        <y>85.42109840411501</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>28</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>130.99357336573604</x>
        <y>33.563347799757125</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>29</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>46.890570472099824</x>
        <y>84.32697531265379</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>30</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>289.29241608338094</x>
        <y>79.10614026359546</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>31</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>100.85049907610703</x>
        <y>29.219819221326194</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>32</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>93.66013534793747</x>
        <y>61.22227570233571</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>33</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>165.39189836567348</x>
        <y>48.74735797514156</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>34</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>18.853444997565738</x>
        <y>6.082388970997076</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>35</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>259.5180066895893</x>
        <y>75.51462617878758</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>36</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>263.7950489517294</x>
        <y>90.09995862170234</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>37</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>23.947500697143653</x>
        <y>94.74616081134577</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>38</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>241.77318785378117</x>
        <y>91.62879072642055</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>39</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>66.62200995388741</x>
        <y>32.556745277962186</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>40</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>44.26079431121239</x>
        <y>46.605254676089366</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>41</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>194.44814750115458</x>
        <y>79.42937060855046</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>42</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>183.8414711646846</x>
        <y>99.24659864419542</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>43</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>255.80325337307795</x>
        <y>89.00191251557604</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>44</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>3.9615742093764172</x>
        <y>21.929477393662957</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>45</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>263.8017987770105</x>
        <y>49.45572112660953</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>46</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>177.29759773129527</x>
        <y>10.061128779807616</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>47</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>65.42708077018108</x>
        <y>78.7624915799955</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>48</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>13.61768418807834</x>
        <y>49.54522480122073</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>49</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>274.0951558609378</x>
        <y>65.79963370698627</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>50</id>
      </interface_config>
      <motetype_identifier>mtype98</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>262</width>
    <z>3</z>
    <height>185</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <viewport>1.283542488892569 0.0 0.0 1.283542488892569 57.05308221384719 36.88829601722232</viewport>
      <hidden />
    </plugin_config>
    <width>496</width>
    <z>0</z>
    <height>198</height>
    <location_x>1</location_x>
    <location_y>184</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>497</width>
    <z>1</z>
    <height>333</height>
    <location_x>0</location_x>
    <location_y>381</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(600000, log.log(last_stats));

NR_NODES = mote.getSimulation().getMotesCount();
MIN_PACKETS = 8;
MAX_PACKETS = 15;/* seq is 4 bits */

function
print_stats()
{
  stats = "";
  stats += "---- " + total_received  + " messages, " +
      doubleFormat.format(total_received / NR_NODES) + " messages/node, " +
      total_dups + " dups, " +
      doubleFormat.format(total_dups / NR_NODES) + " dups/node, " +
      doubleFormat.format(total_hops / total_received) + " hops/message\n";

  stats += "Seqnos:    ";
  for(i = 1; i &lt;= NR_NODES; i++) {
    stats += integerFormat.format(seqnos[i]) + " ";
  }
  stats += "\n";
  stats += "Dups:        ";
  for(i = 1; i &lt;= NR_NODES; i++) {
    stats += integerFormat.format(dups[i]) + " ";
  }
  stats += "\n";
  stats += "Hops:        ";
  for(i = 1; i &lt;= NR_NODES; i++) {
    stats += integerFormat.format(hops[i]) + " ";
  }
  stats += "\n";
  stats += "Received: ";
  for(i = 1; i &lt;= NR_NODES; i++) {
    stats += integerFormat.format(count[i]) + " ";
  }
  stats += "\n";
  return stats;
}

/* Init */
booted = new Array();
count = new Array();
hops = new Array();
seqnos = new Array();
dups = new Array();
total_received = 0;
total_hops = 0;
total_dups = 0;
last_total_received = 0;
last_total_dups = 0;
last_stats = "No stats collected!\n";
doubleFormat = new java.text.DecimalFormat("0.00")
integerFormat = new java.text.DecimalFormat("00")
for(i = 1; i &lt;= NR_NODES; i++) {
  booted[i] = false;
  count[i] = 0;
  hops[i] = 0;
  seqnos[i] = -1;
  dups[i] = 0;
}

/* Wait until all nodes have started */
log.log("Simulation has " + NR_NODES + " nodes\n");
for(i = 1; i &lt;= NR_NODES; i++) {
  YIELD_THEN_WAIT_UNTIL(msg.startsWith('Starting'));
  /*log.log("Node " + id + " booted\n");*/
}

/* Create sink */
log.log("All nodes booted, creating sink at node " + id + "\n\n");
mote.getInterfaces().getButton().clickButton()

GENERATE_MSG(5000, "PERIODIC_STATUS");
log.log("LINE:5s, DOT:sink packet (excl. dups), COMMA:sink dup\n");

while(true) {
  YIELD();

  /* Count sensor data packets */
  if(msg.startsWith("Sink got message")) {
    node_text = msg.split(" ")[4];
    seqno_text = msg.split(" ")[6];
    hops_text = msg.split(" ")[8];
    if(node_text) {
      source = parseInt(node_text);
      seqno = parseInt(seqno_text);
      hop = parseInt(hops_text);
      if (seqno &gt; seqnos[source]) {
        seqnos[source] = seqno;
        count[source]++;
        hops[source] = hop;
        total_received++;
        total_hops += hop;
      } else {
        /*log.log("Warning: Duplicate: " + msg + "\n");*/
        dups[source] += 1;
        total_dups += 1;
      }
    }

    /* Fail if the sink has received more than MAX_PACKETS messages from any node. */
    for(i = 1; i &lt;= NR_NODES; i++) {
      if(count[i] &gt;= MAX_PACKETS) {
        log.log(print_stats());
        log.testFailed(); /* We are done! */
      }
    }

    /* Success if the sink has received more than MIN_PACKETS messages from all nodes */
    for(i = 1; i &lt;= NR_NODES; i++) {
      if(count[i] &lt; MIN_PACKETS) {
        break;
      }
      if(i == NR_NODES) {
        log.log(print_stats());
        log.testOK();
      }
    }
    /*print_stats();*/
    last_stats = print_stats();
  }

  if (msg.equals("PERIODIC_STATUS")) {
    diff = total_received - last_total_received;
    last_total_received = total_received;
    diffStr = "";
    while (diff &gt; 0) {
      diff--;
      diffStr += ".";
    }
    log.log(diffStr);

    diff = total_dups - last_total_dups;
    last_total_dups = total_dups;
    diffStr = "";
    while (diff &gt; 0) {
      diff--;
      diffStr += ",";
    }
    log.log(diffStr);

    log.log("\n");

    GENERATE_MSG(5000, "PERIODIC_STATUS");
  }
}</script>
      <active>true</active>
    </plugin_config>
    <width>676</width>
    <z>2</z>
    <height>714</height>
    <location_x>497</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

