PUBLIC_IMAGE_V0=public-images-registry-vpc.cn-hangzhou.cr.aliyuncs.com/public/sf-ai-2023:judge
PUBLIC_IMAGE_WEB=public-images-registry-vpc.cn-hangzhou.cr.aliyuncs.com/public/sf-ai-2023:web
all:
	docker buildx build --platform=linux/arm64,linux/amd64 -f Dockerfile-sf -t $(PUBLIC_IMAGE_V0) --progress=plain . --push
web:
	docker buildx build --platform=linux/arm64,linux/amd64 -f Dockerfile-web -t $(PUBLIC_IMAGE_WEB) --progress=plain .. --push
test:
	docker container run --network host -it --rm ubuntu-sf
push:
	docker tag ubuntu-sf public-images-registry-vpc.cn-hangzhou.cr.aliyuncs.com/public/sf-ai-2023:judge
	docker push public-images-registry-vpc.cn-hangzhou.cr.aliyuncs.com/public/sf-ai-2023:judge
