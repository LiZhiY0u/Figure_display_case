const DEVICE_NAME = 'FigureCase'
const SERVICE_UUID = '0000FF10-0000-1000-8000-00805F9B34FB'
const WRITE_UUID = '0000FF11-0000-1000-8000-00805F9B34FB'

Page({
  data: {
    connected: false,
    connecting: false,
    statusText: '未连接'
  },

  deviceId: '',

  onLoad() {
    wx.onBLEConnectionStateChange(({ deviceId, connected }) => {
      if (deviceId !== this.deviceId) return

      this.setData({
        connected,
        connecting: false,
        statusText: connected ? '已连接' : '连接已断开'
      })
    })
  },

  onUnload() {
    this.closeConnection()
    wx.closeBluetoothAdapter()
  },

  toggleConnection() {
    if (this.data.connected) {
      this.closeConnection()
      return
    }

    this.connect()
  },

  async connect() {
    this.setData({ connecting: true, statusText: '正在初始化蓝牙…' })

    try {
      await this.openAdapter()
      this.setData({ statusText: '正在搜索 FigureCase…' })
      const deviceId = await this.findDevice()
      await this.createConnection(deviceId)
      await this.verifyWriteCharacteristic(deviceId)

      this.deviceId = deviceId
      this.setData({
        connected: true,
        connecting: false,
        statusText: '已连接'
      })
    } catch (error) {
      this.setData({ connected: false, connecting: false, statusText: '连接失败' })
      wx.showToast({ title: error.errMsg || error.message || '连接失败', icon: 'none' })
    }
  },

  openAdapter() {
    return new Promise((resolve, reject) => {
      wx.openBluetoothAdapter({ success: resolve, fail: reject })
    })
  },

  findDevice() {
    return new Promise((resolve, reject) => {
      let settled = false

      const finish = (callback, value) => {
        if (settled) return
        settled = true
        clearTimeout(timer)
        wx.stopBluetoothDevicesDiscovery()
        wx.offBluetoothDeviceFound(onFound)
        callback(value)
      }

      const onFound = ({ devices }) => {
        const target = devices.find(device =>
          device.name === DEVICE_NAME || device.localName === DEVICE_NAME
        )
        if (target) finish(resolve, target.deviceId)
      }

      const timer = setTimeout(() => {
        finish(reject, new Error('未找到 FigureCase'))
      }, 10000)

      wx.onBluetoothDeviceFound(onFound)
      wx.startBluetoothDevicesDiscovery({
        allowDuplicatesKey: false,
        success: () => {},
        fail: error => finish(reject, error)
      })
    })
  },

  createConnection(deviceId) {
    return new Promise((resolve, reject) => {
      wx.createBLEConnection({ deviceId, timeout: 10000, success: resolve, fail: reject })
    })
  },

  verifyWriteCharacteristic(deviceId) {
    return new Promise((resolve, reject) => {
      wx.getBLEDeviceServices({
        deviceId,
        success: ({ services }) => {
          const service = services.find(item => item.uuid.toUpperCase() === SERVICE_UUID)
          if (!service) {
            reject(new Error('未找到遥控服务'))
            return
          }

          wx.getBLEDeviceCharacteristics({
            deviceId,
            serviceId: service.uuid,
            success: ({ characteristics }) => {
              const writable = characteristics.some(item =>
                item.uuid.toUpperCase() === WRITE_UUID &&
                (item.properties.write || item.properties.writeNoResponse)
              )
              writable ? resolve() : reject(new Error('未找到遥控写入特征值'))
            },
            fail: reject
          })
        },
        fail: reject
      })
    })
  },

  sendKey(event) {
    if (!this.data.connected) {
      wx.showToast({ title: '请先连接设备', icon: 'none' })
      return
    }

    const value = new ArrayBuffer(1)
    new Uint8Array(value)[0] = Number(event.currentTarget.dataset.code)

    wx.writeBLECharacteristicValue({
      deviceId: this.deviceId,
      serviceId: SERVICE_UUID,
      characteristicId: WRITE_UUID,
      value,
      writeType: 'write',
      fail: error => {
        wx.showToast({ title: error.errMsg || '发送失败', icon: 'none' })
      }
    })
  },

  closeConnection() {
    if (this.deviceId) {
      wx.closeBLEConnection({ deviceId: this.deviceId })
    }
    this.deviceId = ''
    this.setData({ connected: false, connecting: false, statusText: '未连接' })
  }
})
