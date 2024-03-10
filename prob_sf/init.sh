set -ex
docker pull public-images-registry.cn-hangzhou.cr.aliyuncs.com/public/sf-ai-2023:web
docker pull public-images-registry.cn-hangzhou.cr.aliyuncs.com/public/sf-ai-2023:judge
docker compose -f docker-compose-dev.yml up -d



#while [ ! -f "node-judge/judgeResult.json"  ]
#do
#     sleep 10
#     echo "wating" `date`
#done







