/**
* http://mailman.alsa-project.org/pipermail/alsa-devel/2016-May/108068.html
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
				MX6QDL_PAD_DISP0_DAT21__AUD4_TXD  0x130b0
				MX6QDL_PAD_DISP0_DAT19__AUD4_RXC  0x130b0
				MX6QDL_PAD_DISP0_DAT23__AUD4_RXD  0x100b0
				MX6QDL_PAD_DISP0_DAT18__AUD4_RXFS 0x110b0
*/

#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <sound/soc.h>

#include "imx-audmux.h"

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

	printk(KERN_ERR "-->imx-audmux-config ext-port port=%#010x ptcr=%#010x pdcr=%#010x\n",
				 master, ptcr, pdcr);
	printk(KERN_ERR "-->imx-audmux-config int-port port=%#010x ptcr=%#010x pdcr=%#010x\n",
				 slave, ptcr, pdcr);

	return 0;
}

static int imx_wilink_hw_params(struct snd_pcm_substream *substream,
                                struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai         *cpu_dai = rtd->cpu_dai;
	u32 channels = params_channels(params);
	u32 rate = params_rate(params);
	u32 bclk = rate * channels * 32;
	int ret = 0;

printk(KERN_ERR "--> imx-itel-hw-params NAME=%s  \n VALUE=%#010x"
										"SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS\n",
												cpu_dai->name,
												(SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS));

	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
														SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret) {
		dev_err(cpu_dai->dev, "failed to set dai fmt\n");
		return ret;
	}
	printk(KERN_ERR "--> imx-itel-hw-params set tdm slots channels=%d", channels);

	ret = snd_soc_dai_set_tdm_slot(cpu_dai,
	                               channels == 1 ? 0XFFFFFFFE : 0XFFFFFFFC,
	                               channels == 1 ? 0XFFFFFFFE : 0XFFFFFFFC,
	                               2, 32);
	if (ret) {
		dev_err(cpu_dai->dev, "failed to set dai tdm slot\n");
		return ret;
	}

	printk(KERN_ERR "--> imx-audmux-config set sysclk NAME=%s\n",cpu_dai->name);

	ret = snd_soc_dai_set_sysclk(cpu_dai, 0, bclk, SND_SOC_CLOCK_OUT);
	if (ret) {
			dev_err(cpu_dai->dev, "failed to set sysclk\n");
	}
	return ret;
}

static struct snd_soc_ops imx_wilink_ops = {
		.hw_params = imx_wilink_hw_params,
};

static struct snd_soc_dai_link imx_wilink_dai[] = {
	{
		.name            = "BT HiFi",
		.stream_name     = "Bluetooth Headphone",
		.codec_dai_name  = "snd-soc-dummy-dai",
		.codec_name      = "snd-soc-dummy",
		.ops             = &imx_wilink_ops,
		// .symmetric_rates = 1,
	},
};

static struct snd_soc_card snd_soc_card_imx_wilink = {
	.name       = "imx-audio-bt",
	.dai_link   = imx_wilink_dai,
	.num_links  = ARRAY_SIZE(imx_wilink_dai),
	.owner      = THIS_MODULE,
};


static int imx_wilink_probe(struct platform_device *pdev)
{
	struct snd_soc_card    *card = &snd_soc_card_imx_wilink;
	struct device_node     *ssi_np;
	struct platform_device *ssi_pdev;
	struct device_node     *codec_np;
	// struct imx_wilink_data *data = NULL;
	int int_port, ext_port, ret;

	printk(KERN_ERR "--> imx_wilink_probe");

	ret = of_property_read_u32(pdev->dev.of_node, "mux-int-port", &int_port);
	if (ret) {
		dev_err(&pdev->dev, "mux-int-port missing or invalid\n");
		return ret;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "mux-ext-port", &ext_port);
	if (ret) {
		dev_err(&pdev->dev, "mux-ext-port missing or invalid\n");
		return ret;
	}

	printk(KERN_ERR "--> Configure audmux\n");

	imx_audmux_config(int_port, ext_port);

	/* find value in devicetree for ssi controller */
	ssi_np = of_parse_phandle(pdev->dev.of_node, "ssi-controller", 0);
	if (!ssi_np) {
		dev_err(&pdev->dev, "ssi-controller missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	/* find SSI platform driver */
	ssi_pdev = of_find_device_by_node(ssi_np);
	if (!ssi_pdev) {
		dev_err(&pdev->dev, "failed to find SSI platform device\n");
		ret = -EPROBE_DEFER;
		goto fail;
	}

	codec_np = of_parse_phandle(pdev->dev.of_node, "audio-codec", 0);
	if (!codec_np) {
		dev_err(&pdev->dev, "codec missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	// codec_dev = NULL;
	card->dev = &pdev->dev;
	card->dai_link->cpu_dai_name     = dev_name(&ssi_pdev->dev);
	card->dai_link->platform_of_node = ssi_np;
	card->dai_link->codec_of_node    = codec_np;

	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, &snd_soc_card_imx_wilink);

	printk(KERN_ERR "--> ITEL imx-probe register card\n");

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register card: %d\n", ret);
		goto fail;
	}

	printk(KERN_ERR "--> imx-probe of ssi\n");
	of_node_put(ssi_np);
	printk(KERN_ERR "--> imx-probe of codec\n");
	of_node_put(codec_np);

	return 0;

fail:
	if (ssi_np) {
		of_node_put(ssi_np);
	}
	if (codec_np) {
		of_node_put(codec_np);
	}
	return ret;
}

static int imx_wilink_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_card_imx_wilink;

	snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id imx_wilink_dt_ids[] = {
	{ .compatible = "fsl,imx-audio-bt", },
	{ }
};
MODULE_DEVICE_TABLE(of, imx_wilink_dt_ids);

static struct platform_driver imx_wilink_driver = {
	.driver = {
		.name           = "imx-audio-bt",
		.owner          = THIS_MODULE,
		.pm             = &snd_soc_pm_ops,
		.of_match_table = imx_wilink_dt_ids,
	},
	.probe  = imx_wilink_probe,
	.remove = imx_wilink_remove,
};

module_platform_driver(imx_wilink_driver);

MODULE_AUTHOR("Max Agneni <iptronix.com>");
MODULE_DESCRIPTION("Freescale i.MX WILINK8 ASoC machine driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:imx-wilink8");
