import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
const { exec } = require('node:child_process')
const fs = require('fs')
const path = require('path')
var url = require('url');

let isRunning = false;
let runResult = null;

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  css: {
    preprocessorOptions: {
      less: {
        javascriptEnabled: true,
      },
    },
  },
  server: {
    port: 3000,
    host: true, // Here
    proxy: {
      '^/api': {
        forward: true,
        bypass: (req, res) => {
            if (req.url === '/api/result') {
                const resultPath = path.join(__dirname, '../node-judger/judgeResult.json')
                const isRightMap = fs.existsSync(resultPath)
                if (!isRightMap) {
                    return res.end(JSON.stringify({code: 0, msg: 'ok', data: {extras: {testcases: []}, score: 0}}))
                }
                const result = fs.readFileSync(resultPath, {encoding: 'utf-8'})
                return res.end(JSON.stringify({code: 0, msg: 'ok', data: JSON.parse(result)}))
            } else if (req.url.startsWith('/api/replay')) {
                const parsedUrl = url.parse(req.url)
                const queryMap = (parsedUrl.query || '').split('&').reduce((pre, i) => {
                  const res = i.split('=');
                  if (res.length === 2) {
                    pre[res[0]] = res[1]
                  }
                  return pre
                }, {});
                if (queryMap.map) {
                    const file = path.join(__dirname, `../node-judger/replays/${queryMap.map}-replay.json`)
                    const isRightMap = fs.existsSync(file)
                    if (!isRightMap) {
                        return res.end(JSON.stringify({code: 1, msg: '错误的地图'}))
                    }
                    const result = fs.readFileSync(file, {encoding: 'utf-8'})
                    return res.end(JSON.stringify({code: 0, msg: 'ok', data: JSON.parse(result)}))
                }
                return res.end('error')
            } else if (req.url.startsWith('/api/run')) {
                // if (runResult) {
                //   isRunning = false;
                //   setTimeout(() => runResult = null, 0);
                //   return res.end(runResult);
                // }

                // if (!isRunning) {
                //   isRunning = true;
                //   exec('bash run.sh', {cwd: '../'}, (error) => {
                //     if (error) {
                //       runResult = JSON.stringify({code: -1, data: null, msg: 'error', error, status: 'finish'})
                //     } else {
                //       runResult = JSON.stringify({code: 0, data: null, msg: 'ok', status: 'finish'})
                //     }
                //     isRunning = false;
                //   })
                //   return res.end(JSON.stringify({code: 0, data: null, msg: 'ok', status: 'running'}))
                // } else {
                //   return res.end(JSON.stringify({code: 0, data: null, msg: 'ok', status: 'running'}))
                // }
            }
        }
      }
    }
  }
})
