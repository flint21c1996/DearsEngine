struct PSInput // GS가 있다면 GSInput으로 사용됨
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float opacity : OPACITY;
    unsigned int available : AVAILABLE;        //사용자 정의 시맨틱
};

float4 main(PSInput input) : SV_TARGET
{
 // available 값이 0일 경우 픽셀 폐기
    if (input.available == 0) 
    {
        discard; // 픽셀 셰이더에서 현재 픽셀을 그리지 않음
    }
    return float4(input.color, input.opacity);
}