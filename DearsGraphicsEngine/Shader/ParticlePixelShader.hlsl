struct PSInput // GS�� �ִٸ� GSInput���� ����
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float opacity : OPACITY;
    unsigned int available : AVAILABLE;        //����� ���� �ø�ƽ
};

float4 main(PSInput input) : SV_TARGET
{
 // available ���� 0�� ��� �ȼ� ���
    if (input.available == 0) 
    {
        discard; // �ȼ� ���̴����� ���� �ȼ��� �׸��� ����
    }
    return float4(input.color, input.opacity);
}