/**
* https://community.nxp.com/thread/321837
*
* Device tree entires are below
		sound-bt {
			compatible = "fsl,imx-audio-bt";
			model = "imxwilink-bt";
			ssi-controller = <&ssi2>;
			mux-int-port = <2>;
			mux-ext-port = <5>;
		};
 *
 *Added below to audmux pin control
				MX6QDL_PAD_DISP0_DAT20__AUD4_TXC  0x130b0
				MX6QDL_PAD_DISP0_DAT21__AUD4_TXD  0x110b0
				MX6QDL_PAD_DISP0_DAT22__AUD4_TXFS 0x130b0
				MX6QDL_PAD_DISP0_DAT23__AUD4_RXD  0x130b0
*/

#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <sound/soc.h>

#include "imx-audmux.h"

//#define I2S_MODE_MASTER

#define MASTER_DAI_FMT (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS)
#define SLAVE_DAI_FMT  (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_IB_NF | SND_SOC_DAIFMT_CBM_CFM)

struct imx_bt_data {
	struct snd_soc_dai_link  dai;
	struct snd_soc_card      card;
	struct platform_device  *pdev;
};

static int imx_audmux_config(int slave, int master)
{
	uint32_t ptcr, pdcr;
	int ret;

	slave--;
	master--;

	/*Configure the audmux*/
	ptcr = IMX_AUDMUX_V2_PTCR_SYN |
	       IMX_AUDMUX_V2_PTCR_TFSDIR |
	       IMX_AUDMUX_V2_PTCR_TFSEL(slave) |
	       IMX_AUDMUX_V2_PTCR_TCLKDIR |
	       IMX_AUDMUX_V2_PTCR_TCSEL(slave);
	pdcr = IMX_AUDMUX_V2_PDCR_RXDSEL(slave);

	ret = imx_audmux_v2_configure_port(master, ptcr, pdcr);
	if (ret) {
		printk("ERROR:audmux master port setup failed\n");
		return ret;
	}

	/*
	 * According to RM, RCLKDIR and SYN should not be changed at same time.
	 * So separate to two step for configuring this port.
	 */
	ptcr |= IMX_AUDMUX_V2_PTCR_RFSDIR |
	        IMX_AUDMUX_V2_PTCR_RFSEL(slave) |
	        IMX_AUDMUX_V2_PTCR_RCLKDIR |
	        IMX_AUDMUX_V2_PTCR_RCSEL(slave);
	ret = imx_audmux_v2_configure_port(master, ptcr, pdcr);
	if (ret) {
		printk("ERROR:audmux master port setup failed\n");
		return ret;
	}

	ptcr = IMX_AUDMUX_V2_PTCR_SYN;
	pdcr = IMX_AUDMUX_V2_PDCR_RXDSEL(master);

	ret = imx_audmux_v2_configure_port(slave, ptcr, pdcr);
	if (ret) {
		printk("ERROR:audmux master port setup failed\n");
		return ret;
	}

}

static int imx_bt_audio_probe(struct platform_device *pdev)
{
	struct device_node     *ssi_np, *np = pdev->dev.of_node;
	struct imx_bt_data     *data;
	struct platform_device *ssi_pdev;
	uint32_t ptcr, pdcr;
	int ret = 0, int_port, ext_port;

	/*
		Get the int and ext port from the dtb
	*/
	ret = of_property_read_u32(np, "mux-int-port", &int_port);
	if (ret) {
		dev_err(&pdev->dev, "mux-int-port missing or invalid\n");
		return ret;
	}
	ret = of_property_read_u32(np, "mux-ext-port", &ext_port);
	if (ret) {
		dev_err(&pdev->dev, "mux-ext-port missing or invalid\n");
		return ret;
	}
#ifdef I2S_MODE_MASTER
	imx_audmux_config(int_port, ext_port);
#else
	imx_audmux_config(ext_port, int_port);
#endif

	//Get the i2s controller from dtb
	ssi_np = of_parse_phandle(pdev->dev.of_node, "ssi-controller", 0);
	if (!ssi_np) {
		dev_err(&pdev->dev, "phandle missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	//find the i2s node and get its pdev struct
	ssi_pdev = of_find_device_by_node(ssi_np);
	if (!ssi_pdev) {
		dev_err(&pdev->dev, "failed to find SSI platform device\n");
		ret = -EINVAL;
		goto fail;
	}

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		dev_err(&pdev->dev, "failed to allocate memory\n");
		ret = -ENOMEM;
		goto fail;
	}

	data->dai.name             = "BT HiFi";
	data->dai.stream_name      = "Bluetooth Headphone";
	data->dai.codec_dai_name   = "snd-soc-dummy-dai";
	data->dai.codec_name       = "snd-soc-dummy";
	data->dai.cpu_of_node      = ssi_np;
	data->dai.platform_of_node = ssi_np;
#ifdef I2S_MODE_MASTER
	data->dai.dai_fmt          = MASTER_DAI_FMT;
#else
	data->dai.dai_fmt          = SLAVE_DAI_FMT;
#endif

	data->card.dev       = &pdev->dev;
	data->card.num_links = 1;
	data->card.dai_link  = &data->dai;

	data->pdev = platform_device_register_simple("bluetooth", -1, NULL, 0);
	if (IS_ERR(data->pdev)) {
		ret = PTR_ERR(data->pdev);
		dev_err(&pdev->dev, "register failed: %d\n", ret);
		goto fail;
	}

	//Fill in the card details
	ret = snd_soc_of_parse_card_name(&data->card, "model");
	if (ret) {
		goto fail;
	}

	data->card.num_links = 1;
	data->card.owner     = THIS_MODULE;
	data->card.dai_link  = &data->dai;

	platform_set_drvdata(pdev, &data->card);
	snd_soc_card_set_drvdata(&data->card, data);

	ret = snd_soc_register_card(&data->card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed: %d\n", ret);
		goto fail;
	}

	// Done, set the driver data.
	platform_set_drvdata(pdev, data);

fail:
	if (ssi_np) {
		of_node_put(ssi_np);
	}
	return ret;
}

static int imx_bt_audio_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct imx_bt_data  *data = snd_soc_card_get_drvdata(card);

	snd_soc_unregister_card(card);
	platform_device_unregister(data->pdev);

	return 0;
}

static const struct of_device_id imx_bt_dt_ids[] = {
	{ .compatible = "fsl,imx-audio-bt", },
	{ }
};
MODULE_DEVICE_TABLE(of, imx_bt_dt_ids);

static struct platform_driver imx_bt_driver = {
	.driver = {
		.name           = "imx-audio-bt",
		.owner          = THIS_MODULE,
		.pm             = &snd_soc_pm_ops,
		.of_match_table = imx_bt_dt_ids,
	},
	.probe  = imx_bt_audio_probe,
	.remove = imx_bt_audio_remove,
};

module_platform_driver(imx_bt_driver);

MODULE_AUTHOR("Max Agneni <iptronix.com>");
MODULE_DESCRIPTION("Freescale i.MX WILINK8 ASoC machine driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:imx-wilink8");
