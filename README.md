# **power_consumption_trace**

compile.sh << gcc용 컴파일 명령어
del_cache << 페이지 캐시 삭제용
record.exe << 실행시 즉시 실행 시작, ctl+c로 중단하면 파일작성 중단됨. 파일은 /data에 저장됨
measuring.c << power trace source code, 수정해도됨. thread로 sampling 간격을 조절하고 있는데 아마 지금 10ms마다 기록중일거임

1. measuring.c의 path들을 수정하고 저장
2. complie.sh 실행
3. record.exe 실행해서 data에 파일 생성되는지 확인
> 작성된 데이터 확인해보면 첫번째 줄이 각 power component고 두번째줄부터 바로 power 측정값
> total power는 6개 데이터를 다 합치면 됨
