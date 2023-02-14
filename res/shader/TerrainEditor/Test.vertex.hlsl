struct VSOut
{
    float4 pos : SV_Position;
};

VSOut vs_main()
{
    VSOut result = {
        float4(0.0, 0.0, 0.0, 1.0)
    };
    return result;
}

typedef VSOut PSin;

struct PSOut
{
    float4 color : SV_Target0;
};

PSOut ps_main(PSin input)
{
    PSOut result;

    return result;
}
