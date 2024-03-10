set -ex

echo "0-Clean"
cd `dirname $0`
docker compose -f docker-compose-judge.yml down
rm -rf node-player node-judger build || true

tar -czf code.tar.gz --directory=code .

echo "2-Build"
mkdir build
tar -xzf code.tar.gz --director=build

echo "3-Run"
cd `dirname $0`
cp -rp build node-player

cd `dirname $0`
cp -rp judge-server node-judger

rm -rf node-judger/judgeResult.json


docker compose -f docker-compose-judge.yml down
docker compose -f docker-compose-judge.yml up



#while [ ! -f "node-judge/judgeResult.json"  ]
#do
#     sleep 10
#     echo "wating" `date`
#done







