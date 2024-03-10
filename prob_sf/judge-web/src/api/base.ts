// Some imports not used depending on template conditions
import globalAxios, { AxiosInstance } from 'axios'

const instance: AxiosInstance = globalAxios.create({
    baseURL: '/api'
})

export default instance
