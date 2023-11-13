n_elec = 8;
n_rings= 1;
options = {'no_meas_current','no_rotate_meas'};
amplitude=5; %amplitude of stimulation current, default=10mA
[st, ms]= mk_stim_patterns(n_elec, n_rings, '{ad}','{ad}', options, amplitude);
inv_model= mk_common_model('g2c0',n_elec); %1024 elementi
inv_model.fwd_model.stimulation = st;
inv_model.fwd_model.meas_select = ms;

img=inv_solve(inv_model, baseline, h6);
K2=max(img.elem_data,[],"all");
K1=min(img.elem_data,[],"all");
colbar = linspace(K1,K2,10);
t_v=colbar; %ticks values
Y = (K2-K1)./2;
X = (K2+K1)./2; 

subplot(2,3,1)
img1=inv_solve(inv_model, baseline, h1);
show_fem(img1,[1,1]) 
img1.calc_colours.ref_level= X;
img1.calc_colours.clim= Y;
cb = colorbar; 
t_l=linspace(1,255,10);
set(cb,'YTick', t_l'); set(cb,'YTickLabel', t_v');

subplot(2,3,2)
img1=inv_solve(inv_model, baseline, h2);
show_fem(img1,[1,1]) 
img1.calc_colours.ref_level= X;
img1.calc_colours.clim= Y;
cb = colorbar; 
t_l=linspace(1,255,10);
set(cb,'YTick', t_l'); set(cb,'YTickLabel', t_v');

subplot(2,3,3)
img1=inv_solve(inv_model, baseline, h3);
show_fem(img1,[1,1]) 
img1.calc_colours.ref_level= X;
img1.calc_colours.clim= Y;
cb = colorbar; 
t_l=linspace(1,255,10);
set(cb,'YTick', t_l'); set(cb,'YTickLabel', t_v');

subplot(2,3,4)
img1=inv_solve(inv_model, baseline, h4);
show_fem(img1,[1,1]) 
img1.calc_colours.ref_level= X;
img1.calc_colours.clim= Y;
cb = colorbar; 
t_l=linspace(1,255,10);
set(cb,'YTick', t_l'); set(cb,'YTickLabel', t_v');

subplot(2,3,5)
img1=inv_solve(inv_model, baseline, h5);
show_fem(img1,[1,1]) 
img1.calc_colours.ref_level= X;
img1.calc_colours.clim= Y;
cb = colorbar; 
t_l=linspace(1,255,10);
set(cb,'YTick', t_l'); set(cb,'YTickLabel', t_v');

subplot(2,3,6)
img1=inv_solve(inv_model, baseline, h6);
show_fem(img1,[1,1]) 
img1.calc_colours.ref_level= X;
img1.calc_colours.clim= Y;
cb = colorbar; 
t_l=linspace(1,255,10);
set(cb,'YTick', t_l'); set(cb,'YTickLabel', t_v');

%% plot singoli
img=inv_solve(inv_model, baseline, h6);
title(['10.5 cm'])
show_fem(img,[1,1])
img.calc_colours.ref_level= X;
img.calc_colours.clim= Y;
cb = colorbar; 
t_l=linspace(1,255,10);
set(cb,'YTick', t_l'); set(cb,'YTickLabel', t_v');

%% dati
img=inv_solve(inv_model, baseline, h6);
K2=max(img.elem_data,[],"all");
disp(K2)

