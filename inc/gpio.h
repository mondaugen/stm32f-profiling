#ifndef GPIO_H
#define GPIO_H 

extern int numClicks;

void gpio_setup(void);
void gpio_blue_button(void);
void gpio_my_button(void);
void gpio_setup_pull_down(void);
#endif /* GPIO_H */
